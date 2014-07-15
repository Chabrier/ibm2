/*
 * @file vle/gvle/global/IbmInsideGVLE/PluginIbmInsideGVLE.cpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2011 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2011 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vle/gvle/GVLE.hpp>
#include <vle/gvle/GVLEMenuAndToolbar.hpp>
#include <vle/gvle/FileTreeView.hpp>
#include <vle/gvle/Message.hpp>
#include <vle/gvle/Editor.hpp>
#include <vle/gvle/Modeling.hpp>
#include <vle/utils/Package.hpp>
#include <gtkmm/widget.h>
#include <gtkmm/menubar.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/dialog.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/button.h>
#include <gtkmm/action.h>
#include <gtkmm/comboboxtext.h>
#include <vle/utils/Spawn.hpp>
#include <vle/utils/Template.hpp>
#include <vle/vpz/Classes.hpp>
#include <vle/vpz/Class.hpp>
#include <vle/vpz/BaseModel.hpp>
#include <fstream>
#include <iostream>

#include <vle/vpz/AtomicModel.hpp>
#include <vle/gvle/ModelingPlugin.hpp>
#include <vle/vpz/Class.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace vle {
namespace gvle {
namespace global {
namespace ibminsidegvle {

typedef std::vector < std::string > strings_t;

const Glib::ustring UI_DEFINITION =
    "<ui>"
    "    <popup name='FileTV_Popup'>"
    "        <menuitem name='FileTV_ContextLaunchIbm' action='FileTV_ContextLaunchIbm'/>"
    "    </popup>"
    "</ui>";

/**
 * @class PluginIbminsideGVLE
 * @brief testing the global plugin
 *
 */


class PluginIbmInsideGVLE : public GlobalPlugin
{
public:

    /**
     * Class constructor.
     *
     * @param[in] package
     * @param[in] library
     * @param[in/out] gvle : an access to everything
     */
    PluginIbmInsideGVLE(const std::string& package,
                 const std::string& library,
                 vle::gvle::GVLE* gvle):
        GlobalPlugin(package, library, gvle)
    {
    }

    /**
     * Class destructor.
     */
    virtual ~PluginIbmInsideGVLE()
    {

        mConnection.disconnect();

        m_refActionGroup->remove(m_refActionGroup->get_action("FileTV_ContextLaunchIbm"));
        m_refUIManager->remove_ui(mMergeId);

		if (mClasses) {
        	mClasses->remove_all_columns();
    	}

    	for (std::list < sigc::connection >::iterator it = mList.begin();
    	     it != mList.end(); ++it) {
    	    it->disconnect();
    	} 
        
    }

    /**
     * Run
     */
    void run()
    {
        m_refUIManager = mGVLE->getFileTreeView()->getPopupUIManager();
        m_refActionGroup = mGVLE->getFileTreeView()->getPopupActionGroup();

        createNewActions();

        updateUI();
    }

	/**
	* Add the item Open Ibm in the menu
	*/
    void createNewActions()
    {
		m_refActionGroup->add(Gtk::Action::create("FileTV_ContextLaunchIbm", _("_Open Ibm")),
	  sigc::mem_fun(*this, &PluginIbmInsideGVLE::onLaunchIbm));
		
    }


    void updateUI()
    {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
        try {
            mMergeId = m_refUIManager->add_ui_from_string(UI_DEFINITION);
        } catch(const Glib::Error& ex) {
            std::cerr << "building menus failed: " <<  ex.what();
        }
#else
        std::auto_ptr<Glib::Error> ex;
        mMergeId = m_refUIManager->add_ui_from_string(UI_DEFINITION, ex);
        if(ex.get()) {
            std::cerr << "building menus failed: " <<  ex->what();
        }
#endif //GLIBMM_EXCEPTIONS_ENABLED
    }

	/*
	* Open the plugin window, the vpz selected and show the class list
	*/
	void onLaunchIbm()
	{
		Glib::RefPtr< Gtk::Builder > mXml = Gtk::Builder::create();

		vle::utils::Package pack("IbminsideGVLE");
    	std::string glade = pack.getPluginGvleModelingFile(
            "ibmGVLE.glade", vle::utils::PKG_BINARY);
		mXml->add_from_file(glade);

		mXml->get_widget("window1", mWindow);
		mWindow->show();
		//openVpz();
		if (openVpz())
		{
            // //Return vpz
            mXml->get_widget("treeviewClassName", mClasses);
            mClassesListStore = Gtk::ListStore::create(mClassesColumns);
            mClasses->set_model(mClassesListStore);
            initClassesColumnName();
            mClassesCopy = mGVLE->getModeling()->vpz().project().classes(); //Crash here after new model and close if TopModel
            //mClassesCopy = mGVLE->getModeling()->getClassModel();
            fillClasses();
			initMenuPopupClasses();
        
		    mXml->get_widget("buttonApply", mButtonApply);
		    mButtonApply->signal_clicked().connect(sigc::mem_fun(*this,
                                                     &PluginIbmInsideGVLE::onApply));
	        mXml->get_widget("buttonCancel", mButtonApply);
		    mButtonApply->signal_clicked().connect(sigc::mem_fun(*this,
                                                     &PluginIbmInsideGVLE::onCancel));
            createControleurAndDyn("Controleur");
    	}
	}

	void onApply()
	{
        std::cout << "onapply" << std::endl;
        //mAtomicModel->setParent(mGVLE->getModeling()->getTopModel());
		//mGVLE->getModeling()->getClassModel() = mClassesCopy; //Ajout d'un accesseur l152 in Modeling
        mGVLE->getModeling()->vpz().project().classes() = mClassesCopy;
        std::set<std::string>::const_iterator 
            sit (mDeletedClasses.begin()), 
            send(mDeletedClasses.end());
        for (;sit!=send;++sit) {
            if (mGVLE->getModeling()->vpz().project().dynamics().exist(*sit))
            mGVLE->getModeling()->vpz().project().dynamics().del(*sit);
        }

        mDeletedClasses.clear();
        mClassesCopy.clear(); 
        mGVLE->getModeling()->setModified(true);

		mWindow->hide();
        mGVLE->redrawModelClassBox();
        //Refresh models and classes
        mGVLE->onSave();
	}

	void onCancel()
	{
		mWindow->hide();
	}

    /*
    * Open the Vpz selected on the TreeView
    */
	bool openVpz()
	{
		FileTreeView* f = mGVLE->getFileTreeView();
	    std::string vpz_file = f->getSelectedVpz();

		if (utils::Path::extension(vpz_file) == ".vpz")
		{
			mGVLE->parseXML(vpz_file);
			mGVLE->getEditor()->openTabVpz(mGVLE->getModeling()->getFileName(),
		                                mGVLE->getModeling()->getTopModel());
			mGVLE->redrawModelTreeBox();
		    mGVLE->redrawModelClassBox();
		    mGVLE->getMenu()->onOpenVpz();
		    mGVLE->getModelTreeBox()->set_sensitive(true);
		    mGVLE->getModelClassBox()->set_sensitive(true);
			return true;
		}
        return false;
	}

    void createControleurAndDyn(std::string name) {
        if (!existControleur(name)) {
            createControleur(name);
        }
        if (!existDynControleur(name)) {
            createDynControleur(name);
        }
    }

    bool existControleur(std::string name) {
        return mGVLE->getModeling()->getTopModel()->exist(name);
    }

    void createControleur(std::string name) {
        vpz::AtomicModel* atomicModel = new vpz::AtomicModel(name, mGVLE->getModeling()->getTopModel());
        atomicModel->setPosition(50, 50);
        atomicModel->setSize(40, 20);
        
        if (existDynControleur(name)) {
            atomicModel->setDynamics("dyn" + name);
        }       
        mGVLE->redrawModelTreeBox();
    }
    
    bool existDynControleur(std::string name) {
        return mGVLE->getModeling()->vpz().project().dynamics().exist("dyn" + name);
    }

    void createDynControleur(std::string name) {
        vpz::Dynamic dyn("dyn" + name);
        dyn.setLibrary(name);
        dyn.setPackage(mGVLE->currentPackage().name());
        mGVLE->getModeling()->vpz().project().dynamics().add(dyn);
        vpz::AtomicModel* atom = mGVLE->getModeling()->getTopModel()->getModel(name)->toAtomic();
        atom->setDynamics(dyn.name());
    }

    /*
    * Print the class list of the vpz selected
    */
	void showListClasses()
	{
		std::map<std::string, vpz::Class> listClass = mClassesCopy.list();
		for (std::map<std::string, vpz::Class>::iterator it=listClass.begin() ; it!=listClass.end() ; ++it)
		{
    		std::cout << "Class name " << it->second.name() << std::endl;
			std::cout << "Model name " << it->second.model()->getName() << std::endl;
		}
	}

    void showListModel()
    {
        std::map<std::string, vpz::BaseModel*> modelList = mGVLE->getModeling()->getTopModel()->getModelList();
        for (std::map<std::string, vpz::BaseModel*>::iterator it=modelList.begin() ; it!=modelList.end() ; ++it)
		{
            std::cout << "first " << it->first << std::endl;
            std::cout << "Model Name " << it->second->getName() << std::endl;
		}
    }
	
    void initClassesColumnName()
    {
        mColumnName = mClasses->append_column_editable(_("Name"),
                                                        mClassesColumns.mName);
        Gtk::TreeViewColumn* nameCol = mClasses->get_column(mColumnName - 1);

        nameCol->set_clickable(true);
        nameCol->set_resizable(true);

        Gtk::CellRendererText* mCellRenderer = dynamic_cast<Gtk::CellRendererText*>(
            mClasses->get_column_cell_renderer(mColumnName - 1));

        mCellRenderer->property_editable() = true;

        mList.push_back(mCellRenderer->signal_editing_started().connect(
                            sigc::mem_fun(*this, &PluginIbmInsideGVLE::onEditionStarted)));
        mList.push_back(mCellRenderer->signal_edited().connect(
                            sigc::mem_fun(*this, &PluginIbmInsideGVLE::onEdition)));
    }


    void onEditionStarted(Gtk::CellEditable* /*celleditable*/,
    const Glib::ustring& path)
    {
        Gtk::TreeModel::Path selectedpath(path);
        Gtk::TreeModel::iterator it = mClassesListStore->get_iter(selectedpath);

        Gtk::TreeModel::Row row = *it;
        if (row) {
            mOldName = row.get_value(mClassesColumns.mName);
        }
    }

    void onEdition(const Glib::ustring& /*pathstring*/,
    const Glib::ustring& newstring)
    {
        std::string newName = newstring.raw();
        boost::trim(newName);
        std::string oldName = mOldName.raw();

        Glib::RefPtr < Gtk::TreeView::Selection > ref = mClasses->get_selection();
        if (ref) {
            Gtk::TreeModel::iterator it = ref->get_selected();
            if (*it and not newName.empty() and newName != oldName) {
                Gtk::TreeModel::Row row = *it;
                if (not mClassesCopy.exist(newName) and isValidName(newName)) {
                    mClassesCopy.rename(mOldName, newName);
                    mRenameList.push_back(std::make_pair(oldName, newName));
                    row[mClassesColumns.mName] = newstring;

                } else {
                    row[mClassesColumns.mName] = mOldName;
                }
            }
        }
    }
    
	/*
	* Fill the treeview's rows with the list of classes
	*/
    void fillClasses()
    {
		std::map<std::string, vpz::Class> listClass = mClassesCopy.list();
		for (std::map<std::string, vpz::Class>::iterator it=listClass.begin() ; it!=listClass.end() ; ++it) 
		{
	        Gtk::TreeIter iter = mClassesListStore->append();
	        if (iter)
			{
	            Gtk::ListStore::Row row = *iter;
	            row[mClassesColumns.mName] = it->first;
	        }
	    }
    	mIter = mClassesListStore->children().end();
    }
    
    void refreshClassesList()
    {
        mClassesListStore->clear();
        fillClasses();
    }

	/*
	* Add or Remove menu with right click
	*/
	void initMenuPopupClasses()
    {
        mActionGroup = Gtk::ActionGroup::create("initMenuPopupClasses");
        mActionGroup->add(Gtk::Action::create("ClasBox_ContextMenu", _("Context Menu")));
    
        mActionGroup->add(Gtk::Action::create("ClasBox_ContextNew", _("_New")),
	    sigc::mem_fun(*this, &PluginIbmInsideGVLE::onAddClasses));
        mActionGroup->add(Gtk::Action::create("ClasBox_ContextModify", _("_Modify")),
	    sigc::mem_fun(*this, &PluginIbmInsideGVLE::onModifyClasses));
        mActionGroup->add(Gtk::Action::create("ClasBox_ContextRemove", _("_Remove")),
	    sigc::mem_fun(*this, &PluginIbmInsideGVLE::onRemoveClass));
        

        mUIManager = Gtk::UIManager::create();
        mUIManager->insert_action_group(mActionGroup);
    
        Glib::ustring ui_info =
	    "<ui>"
	    "  <popup name='ClasBox_Popup'>"
        "      <menuitem action='ClasBox_ContextNew'/>"
        "      <menuitem action='ClasBox_ContextModify'/>"
	    "      <menuitem action='ClasBox_ContextRemove'/>"
	    "  </popup>"
	    "</ui>";

        try {
	        mUIManager->add_ui_from_string(ui_info);
	        mMenu = (Gtk::Menu *) (
	        mUIManager->get_widget("/ClasBox_Popup"));
        } catch(const Glib::Error& ex) {
	        std::cerr << "building menus failed: ClasBox_Popup " <<  ex.what();
        }

        if (!mMenu)
            std::cerr << "menu not found : ClasBox_Popup \n";
    
        mList.push_back(mClasses->signal_button_release_event().connect(
                        sigc::mem_fun(*this,
                                      &PluginIbmInsideGVLE::onButtonRealeaseClasses)));
    }

/*    void onNewClasses() {
        std::string modelName;

        Gtk::TreeIter iter = mClassesListStore->append();
        if (iter) {

            std::string modelName = "newClasses";
            int copyNumber = 1;
            std::string suffixe;
            while (mClassesCopy.exist(modelName)) {
                suffixe = "_" + boost::lexical_cast < std::string >(copyNumber);
                modelName = "newClasses" + suffixe;

                copyNumber++;
            };

            vpz::Class& new_class = mClassesCopy.add(modelName);

            Gtk::ListStore::Row row = *iter;
            row[mClassesColumns.mName] = modelName;

            Gtk::TreeViewColumn* nameCol = mClasses->get_column(mColumnName - 1);
            mClasses->set_cursor(mClassesListStore->get_path(iter),*nameCol,true);

//bouton.signal_clicked().connect(sigc::bind<std::string>(sigc::mem_fun(fenetre, &Gtk::Window::set_title)
            //mClasses->signal_cursor_changed().connect(sigc::bind<vpz::Class&>(sigc::mem_fun(*this, &PluginIbmInsideGVLE::onAddClasses), new_class));
        }
    }*/


	void onAddClasses()
	{
		std::cout << "Add Class" << std::endl;
        std::string modelName;

        Gtk::TreeIter iter = mClassesListStore->append();
        if (iter) {

            std::string modelName = "newClasses";
            int copyNumber = 1;
            std::string suffixe;
            while (mClassesCopy.exist(modelName)) {
                suffixe = "_" + boost::lexical_cast < std::string >(copyNumber);
                modelName = "newClasses" + suffixe;

                copyNumber++;
            };

            vpz::Class& new_class = mClassesCopy.add(modelName);

            Gtk::ListStore::Row row = *iter;
            row[mClassesColumns.mName] = modelName;

            Gtk::TreeViewColumn* nameCol = mClasses->get_column(mColumnName - 1);
            mClasses->set_cursor(mClassesListStore->get_path(iter),*nameCol,true);
        
            

            const std::string pluginname = "vle.forrester/Forrester";
            ModelingPluginPtr plugin = mGVLE->pluginFactory().getModelingPlugin(pluginname, mGVLE->currentPackage().name());
            vpz::Conditions& cond = mGVLE->getModeling()->conditions();
            vpz::Dynamic dyn(new_class.name());
            vpz::Observables& obs = mGVLE->getModeling()->observables();
            mAtomicModel = new vpz::AtomicModel(new_class.name(), NULL);//mGVLE->getModeling()->getTopModel()
            //mAtomicModel->setPosition(50, 50);
            //std::string classname = modelName;
            const std::string namespace_ = "IbminsideGVLE";

            if (plugin->create(*mAtomicModel, dyn, cond, obs, new_class.name(), namespace_))
            {
                const std::string& buffer = plugin->source();
                std::string filename = mGVLE->getPackageSrcFile(new_class.name() + ".cpp");

                //vpz::Class& new_class = mClassesCopy.add(classname);
                new_class.setModel(mAtomicModel);
                dyn.setLibrary(new_class.name());
                dyn.setPackage(mGVLE->currentPackage().name());
                mGVLE->getModeling()->vpz().project().dynamics().add(dyn);
                mAtomicModel->setDynamics(dyn.name());
                try {
                    std::ofstream f(filename.c_str());
                    f.exceptions(std::ofstream::failbit | std::ofstream::badbit);
                    f << buffer;
                } catch(const std::ios_base::failure& e) {
                    throw utils::ArgError(fmt(
                            _("Cannot store buffer in file '%1%'")) % filename);
                }
                refreshClassesList();
            
}
        }
	}

    /*
    * Modify the selected class
    */
    void onModifyClasses()
    {
        Glib::RefPtr < Gtk::TreeView::Selection > ref = mClasses->get_selection();
		if (ref) {
		    Gtk::TreeModel::iterator iter = ref->get_selected();
		    if (iter) {
                Gtk::TreeModel::Row row = *iter;
                std::string name(row.get_value(mClassesColumns.mName));

                std::cout << "Modify Class" << std::endl;
                //const std::string pluginname = "vle.forrester/Forrester";

                //ModelingPluginPtr plugin = mGVLE->pluginFactory().getModelingPlugin(pluginname, mGVLE->currentPackage().name());

                //Trouver modèle atomique, dyn...
                //mAtomicModel = mGVLE->getModeling()->vpz().project().
                //vpz::Dynamic dyn = mGVLE->getModeling()->dynamics().get(name);
std::cout << name << std::endl;
                vpz::Dynamic dyn = mGVLE->getModeling()->vpz().project().dynamics().get(name);
            
                vpz::Conditions& cond = mGVLE->getModeling()->experiment().conditions();
                vpz::Observables& obs = mGVLE->getModeling()->observables();
                vpz::Class& my_class = mClassesCopy.get(name);
                mAtomicModel = my_class.model()->toAtomic();

                std::string filename = mGVLE->getPackageSrcFile(name + ".cpp");
std::cout << "filename " << filename << std::endl;
           /************************************************************/
//filename /home/gcicera/packages/tpForrester/pkgs/tpForrester/src/lol.cpp
///home/gcicera/packages/tpForrester/pkgs/tpForrester/src/seq.cpp
//std::transform(filename.begin(), filename.end(),
	//	       filename.begin(), tolower);
std::string newTab = filename;
          //  try {
                std::string pluginname, packagename, conf;
                utils::Template tpl;
std::cout << "open" << std::endl;
                tpl.open(newTab);
std::cout << "tag" << std::endl;
                tpl.tag(pluginname, packagename, conf);
                ModelingPluginPtr plugin =
                    mGVLE->pluginFactory().getModelingPlugin(packagename,
                                      pluginname,
                                      mGVLE->currentPackage().name());
std::cout << "plugin" << std::endl;
                if (plugin->modify(*mAtomicModel, dyn, cond,
                                   obs, conf, tpl.buffer())) {
std::cout << "plugin source" << std::endl;
                    const std::string& buffer = plugin->source();
                    std::string filename = mGVLE->getPackageSrcFile(dyn.library() +
                        ".cpp");
std::cout << "try into" << std::endl;
                    try {
                        std::ofstream f(filename.c_str());
                        f.exceptions(std::ofstream::failbit |
                                     std::ofstream::badbit);
                        f << buffer;
                    } catch(const std::ios_base::failure& e) {
                        throw utils::ArgError(fmt(
                                _("Cannot store buffer in file '%1%'")) %
                            filename);
                    }
                    //mParent->refresh();
                }
          /*  } catch(...) {
                //mParent->on_apply();
                //mGVLE->getEditor()->openTab(newTab);
            std::cout << "error in try" << std::endl;
            }*/

/************************************************************************/
            }
        }
    }

    /*
    * Remove the selected class
    */
	void onRemoveClass()
	{
		Glib::RefPtr < Gtk::TreeView::Selection > ref = mClasses->get_selection();
		if (ref) {
		    Gtk::TreeModel::iterator iter = ref->get_selected();
		    if (iter) {
		        Gtk::TreeModel::Row row = *iter;
		        std::string name(row.get_value(mClassesColumns.mName));
				std::cout << "name to remove : " << name << std::endl;
		        mClassesCopy.del(name);
		        mClassesListStore->erase(iter);
		        mDeletedClasses.insert(name);
                //Delete the .cpp too ?
		        /*Gtk::TreeModel::Children children = mClassesListStore->children();
		        mIter = children.begin();
		        if (mIter != children.end()) {
		            row = *mIter;
		            Gtk::TreeModel::Path path = mClassesListStore->get_path(mIter);
		            mClasses->set_cursor(path);
		        }*/
		    }
		}
	}

	bool onButtonRealeaseClasses(GdkEventButton* event)
	{
	    if (event->button == 3) {
	        mMenu->popup(event->button, event->time);
	    }
	    return true;
	}

    Glib::RefPtr< Gtk::Action > mAction;
    sigc::connection mConnection;

    Glib::RefPtr<Gtk::UIManager>   m_refUIManager;
    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    Gtk::UIManager::ui_merge_id mMergeId;
	
    Glib::RefPtr<Gtk::Builder> mXml;
    Gtk::Window* mWindow;
    Gtk::TreeView* mClasses;
    Glib::RefPtr<Gtk::ListStore> mClassesListStore;

    struct ClassesModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        ClassesModelColumns()
        { add(mName); }

        Gtk::TreeModelColumn<Glib::ustring> mName;

    } mClassesColumns;

    bool isValidName(std::string name)
    {
        size_t i = 0;
        while (i < name.length()) {
            if (!isalnum(name[i])) {
                if (name[i] != '_') {
                    return false;
                }
            }
            i++;
        }
        return true;
    }

    int mColumnName;
    std::list < sigc::connection > mList;
	Gtk::TreeModel::Children::iterator  mIter;

	Gtk::Menu*                          mMenu;
    Glib::RefPtr <Gtk::UIManager> mUIManager;
    Glib::RefPtr <Gtk::ActionGroup> mActionGroup;

	vpz::Classes                mClassesCopy;
	std::set <std::string>            mDeletedClasses;
	Gtk::Button* mButtonApply;
    vpz::AtomicModel* mAtomicModel;

    Glib::ustring                       mOldName;
    renameList                          mRenameList;
};

}
}
}
}    // namespace vle gvle global rinsidegvle

DECLARE_GVLE_GLOBALPLUGIN(vle::gvle::global::ibminsidegvle::PluginIbmInsideGVLE)
