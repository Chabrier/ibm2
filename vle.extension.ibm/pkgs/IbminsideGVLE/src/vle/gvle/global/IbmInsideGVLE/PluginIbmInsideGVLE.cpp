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
#include <vle/utils/Spawn.hpp>
#include <vle/vpz/Classes.hpp>
#include <vle/vpz/Class.hpp>
#include <iostream>

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
		Glib::RefPtr< Gtk::Builder > xml = Gtk::Builder::create();

		vle::utils::Package pack("IbminsideGVLE");
    	std::string glade = pack.getPluginGvleModelingFile(
            "ibmGVLE.glade", vle::utils::PKG_BINARY);
		xml->add_from_file(glade);

		xml->get_widget("window1", mWindow);
		mWindow->show();
		
		if(openVpz()){
			showListClasses();
		}
	}

    /*
    * Open the Vpz selected on the TreeView
    */
	bool openVpz()
	{
		FileTreeView* f = mGVLE->getFileTreeView();
	    std::string vpz_file = f->getSelectedVpz();

		if (utils::Path::extension(vpz_file) == ".vpz"){
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
    * Print the class list of the vpz opened
    */
	void showListClasses(){
		const vpz::Classes& classes = mGVLE->getModeling()->getClassModel();
		std::map<std::string, vpz::Class> listClass = classes.list();
		for (std::map<std::string, vpz::Class>::iterator it=listClass.begin() ; it!=listClass.end() ; ++it)
		{
    		std::cout << "Class name " << it->second.name() << std::endl; 
			std::cout << "Model name " << it->second.model()->getName() << std::endl;
		}
	}
	
    Glib::RefPtr< Gtk::Action > mAction;
    sigc::connection mConnection;

    Glib::RefPtr<Gtk::UIManager>   m_refUIManager;
    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    Gtk::UIManager::ui_merge_id mMergeId;
	
    Gtk::Window* mWindow;
    Gtk::Button* mButtonForrester;
};

}
}
}
}    // namespace vle gvle global rinsidegvle

DECLARE_GVLE_GLOBALPLUGIN(vle::gvle::global::ibminsidegvle::PluginIbmInsideGVLE)
