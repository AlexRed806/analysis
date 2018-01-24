//
//  SimulAna.cpp
//  
//
//  Created by Alessandro Minotti on 23/01/2018.
//

#include "SimulAna.h"

//#include "</Users/minotti/SuperNEMO/sw/Falaise/install/include/falaise/snemo/datamodels/particle_track_data.h"
#include <datamodels/particle_track_data.h>
#include <bayeux/mctools/simulated_data.h>
#include <bayeux/mctools/base_step_hit.h>

#include <TFile.h>
#include <TTree.h>

DPP_MODULE_REGISTRATION_IMPLEMENT(SimulAna,"SimulAna");

SimulAna::SimulAna() : dpp::base_module() {}

SimulAna::~SimulAna() {
    
    this->reset();
}

void SimulAna::initialize(const datatools::properties & setup_,
                          datatools::service_manager & service_mgr_,
                          dpp::module_handle_dict_type & module_dict_) {
    
    dpp::base_module::_common_initialize(setup_);

    //std::cout << "Module initialisation succesfull" << std::endl;
    
    std::string root_filename = "SimulAna.root";
    if (setup_.has_key("filename")) {
        root_filename = setup_.fetch_string("filename");
        // To allow environment variable
        datatools::fetch_path_with_env(root_filename);
    }
    
    _root_file_ = new TFile(root_filename.c_str(), "RECREATE");
    _root_file_->cd();
    
    _root_tree_ = new TTree("tree", "tree");
    _root_tree_->SetDirectory(_root_file_);
    _root_tree_->Branch("sd.primary_electron_kinetic_energy", &_root_variables_.kinetic_energy);
    _root_tree_->Branch("sd.primary_electron_total_energy", &_root_variables_.total_energy);

    
    this->_set_initialized(true);
    
}


dpp::base_module::process_status SimulAna::process(datatools::things & record_)
{
    //std::cout << "Module process called" << std::endl;
    
    /*if (! record_.has("PTD")) {
        std::cout << "Ain't got PTD" << std::endl;
        return PROCESS_CONTINUE;
    }*/
    
    if (! record_.has("SD")) {
        //std::cout << "Ain't got SD" << std::endl;
        return PROCESS_CONTINUE;
    }
    else {
        
        const mctools::simulated_data & my_sd
        = record_.get<mctools::simulated_data>("SD");
        
        //std::cout << "Event time: " << my_sd.get_time() << std::endl;
        
        const mctools::simulated_data::primary_event_type my_primary_event
        = my_sd.get_primary_event();
        //std::cout << "Event time: " << my_primary_event.get_time() << std::endl;
        
        
        unsigned int N_primary = my_primary_event.get_number_of_particles();
        
        //std::cout << "Number of primary particles: " << N_primary << std::endl;
    
        
        genbb::primary_particle my_primary_particle[N_primary];
        for(unsigned int i_particle=0;i_particle<N_primary;i_particle++) {
            
            my_primary_particle[i_particle] = my_primary_event.get_particle(i_particle);
            //std::cout << "Particle type: " << my_primary_particle[i_particle].get_type() << std::endl;
            
            if(my_primary_particle[i_particle].is_electron()) {
                
                if (get_logging_priority() == datatools::logger::PRIO_TRACE) {
                    DT_LOG_TRACE(get_logging_priority(), "Primary particle is electron");
                    
                    //std::cout << "Electron Ek: " << my_primary_particle[i_particle].get_kinetic_energy() <<      " MeV" << std::endl;
                }
                
                _root_variables_.kinetic_energy = my_primary_particle[i_particle].get_kinetic_energy();
                
                _root_variables_.total_energy = my_primary_particle[i_particle].get_total_energy();
                
                _root_tree_->Fill();
                
            }

        }

    }

    
    return PROCESS_OK;
}

void SimulAna::reset() {
    
    if (_root_file_) {
        // write the output, finished streaming
        _root_file_->cd();
        _root_tree_->Write();
        _root_file_->Close();
        // clean up
        delete _root_file_;
        _root_tree_ = 0;
        _root_file_ = 0;
    }
    
    this->_set_initialized(false);
}
