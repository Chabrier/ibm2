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
#include <vle/vpz/Classes.hpp>
#include <vle/vpz/Class.hpp>
#include <vle/vpz/BaseModel.hpp>
#include <fstream>
#include <iostream>

#include <vle/vpz/AtomicModel.hpp>
#include <vle/gvle/ModelingPlugin.hpp>

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
            //mGVLE->getModeling()->vpz(); //Return vpz
            mXml->get_widget("treeviewClassName", mClasses);
            mClassesListStore = Gtk::ListStore::create(mClassesColumns);
            mClasses->set_model(mClassesListStore);
            initClassesColumnName();
            fillClasses();
			initMenuPopupClasses();
        
		    mXml->get_widget("buttonApply", mButtonApply);
		    mButtonApply->signal_clicked().connect(sigc::mem_fun(*this,
                                                     &PluginIbmInsideGVLE::onApply));
	        mXml->get_widget("buttonCancel", mButtonApply);
		    mButtonApply->signal_clicked().connect(sigc::mem_fun(*this,
                                                     &PluginIbmInsideGVLE::onCancel));
    	}
	}

	void onApply()
	{
		mGVLE->getModeling()->getClassModel() = mClassesCopy; //Ajout d'un accesseur l152 in Modeling
		mGVLE->getModeling()->setModified(true);
		// applyRenaming();
		mWindow->hide();
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

    /*
    * Print the class list of the vpz selected
    */
	void showListClasses()
	{
        mClassesCopy = mGVLE->getModeling()->getClassModel();
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
        std::cout << "onEditionStarted " << path << std::endl;
    }

    void onEdition(const Glib::ustring& /*pathstring*/,
    const Glib::ustring& newstring)
    {
        std::cout << "onEdition " << newstring << std::endl;
    }
    
	/*
	* Fill the treeview's rows with the list of classes
	*/
    void fillClasses()
    {
		mClassesCopy = mGVLE->getModeling()->getClassModel();
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

	/*
	* Add or Remove menu with right click
	*/
	void initMenuPopupClasses()
    {
        mActionGroup = Gtk::ActionGroup::create("initMenuPopupClasses");
        mActionGroup->add(Gtk::Action::create("ClasBox_ContextMenu", _("Context Menu")));
    
        mActionGroup->add(Gtk::Action::create("ClasBox_ContextNew", _("_New")),
	    sigc::mem_fun(*this, &PluginIbmInsideGVLE::onAddClasses));
        mActionGroup->add(Gtk::Action::create("ClasBox_ContextRemove", _("_Remove")),
	    sigc::mem_fun(*this, &PluginIbmInsideGVLE::onRemoveClass));

        mUIManager = Gtk::UIManager::create();
        mUIManager->insert_action_group(mActionGroup);
    
        Glib::ustring ui_info =
	    "<ui>"
	    "  <popup name='ClasBox_Popup'>"
        "      <menuitem action='ClasBox_ContextNew'/>"
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

	void onAddClasses()
	{
		std::cout << "Add Class" << std::endl;
        //PluginName vle.forrester/Forrester0x61a588 
        //classname lol0x61a588
        //namespace tpForrester
        const std::string pluginname = "vle.forrester/Forrester";
        ModelingPluginPtr plugin = mGVLE->pluginFactory().getModelingPlugin(pluginname, mGVLE->currentPackage().name());
		
        //AtomicModelBox atomicModelBox = new AtomicModelBox(mXML, mGVLE->getModeling(), mGVLE);
        //atomicModelBox->show((vpz::AtomicModel*)model);
        vpz::AtomicModel* atomicModel = new vpz::AtomicModel("MyAtomicModel", mGVLE->getModeling()->getTopModel()); 
        /*vpz::Conditions* mCond = new vpz::Conditions(mGVLE->getModeling()->conditions());
        vpz::Dynamics* mDyn = new vpz::Dynamics(mGVLE->getModeling()->dynamics());
        vpz::Observables* mObs = new vpz::Observables(mGVLE->getModeling()->observables());*/
        vpz::Conditions mCond = mGVLE->getModeling()->conditions();
        vpz::Dynamic mDyn("NameOfTheDyn");
        //vpz::Dynamic mDyn = mGVLE->getModeling()->dynamics();
        vpz::Observables mObs = mGVLE->getModeling()->observables();
        const std::string classname = "MyClassName";
        const std::string namespace_ = "IbminsideGVLE";
//create (vpz::AtomicModel &atom, vpz::Dynamic &dynamic, vpz::Conditions &conditions, vpz::Observables &observables, const std::string &classname, const std::string &namespace_)
        if (plugin->create(*atomicModel, mDyn, mCond, mObs, classname, namespace_))
        {
            const std::string& buffer = plugin->source();
            std::string filename = mGVLE->getPackageSrcFile(classname + ".cpp");

            try {
                std::ofstream f(filename.c_str());
                f.exceptions(std::ofstream::failbit | std::ofstream::badbit);
                f << buffer;
            } catch(const std::ios_base::failure& e) {
                throw utils::ArgError(fmt(
                        _("Cannot store buffer in file '%1%'")) % filename);
            }
        }
	}

	void onRemoveClass()
	{
		std::cout << "Remove Class" << std::endl;
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

		        Gtk::TreeModel::Children children = mClassesListStore->children();
		        mIter = children.begin();
		        if (mIter != children.end()) {
		            row = *mIter;
		            Gtk::TreeModel::Path path = mClassesListStore->get_path(mIter);
		            mClasses->set_cursor(path);
		        }
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
    int mColumnName;
    std::list < sigc::connection > mList;
	Gtk::TreeModel::Children::iterator  mIter;

	Gtk::Menu*                          mMenu;
    Glib::RefPtr <Gtk::UIManager> mUIManager;
    Glib::RefPtr <Gtk::ActionGroup> mActionGroup;

	vpz::Classes                mClassesCopy;
	std::set <std::string>            mDeletedClasses;
	Gtk::Button* mButtonApply;

    //Pour ouvrir le plugin
    /*vpz::AtomicModel&                   mAtom;
    vpz::Dynamic&                       mDynamic;
    vpz::Conditions&                    mConditions;
    vpz::Observables&                   mObservables;*/
};

}
}
}
}    // namespace vle gvle global rinsidegvle

DECLARE_GVLE_GLOBALPLUGIN(vle::gvle::global::ibminsidegvle::PluginIbmInsideGVLE)
