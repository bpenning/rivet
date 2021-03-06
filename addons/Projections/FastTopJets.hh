// -*- C++ -*-
#ifndef RIVET_FastTopJets_HH
#define RIVET_FastTopJets_HH

#include "Rivet/Jet.hh"
#include "Rivet/Particle.hh"
#include "Rivet/Projection.hh"
#include "Rivet/Projections/JetAlg.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Tools/RivetFastJet.hh"

#include "fastjet/SISConePlugin.hh"
#include "fastjet/ATLASConePlugin.hh"
#include "fastjet/CMSIterativeConePlugin.hh"
#include "fastjet/CDFJetCluPlugin.hh"
#include "fastjet/CDFMidPointPlugin.hh"
#include "fastjet/D0RunIIConePlugin.hh"
#include "fastjet/TrackJetPlugin.hh"
#include "fastjet/JadePlugin.hh"
//#include "fastjet/PxConePlugin.hh"

namespace Rivet {


  /// Project out jets found using the FastJet package jet algorithms.
  class FastTopJets : public JetAlg {
  public:

    /// Wrapper enum for selected FastJet jet algorithms.
    /// @todo Move to JetAlg and alias here?
    enum JetAlgName { KT, CAM, SISCONE, ANTIKT,
                      // PXCONE,
                      ATLASCONE, CMSCONE,
                      CDFJETCLU, CDFMIDPOINT, D0ILCONE,
                      JADE, DURHAM, TRACKJET };


    /// @name Constructors etc.
    //@{

    /// Constructor from a FastJet JetDefinition
    ///
    /// @warning The AreaDefinition pointer must be heap-allocated: it will be stored/deleted via a shared_ptr.
    FastTopJets(const FinalState& fsp,
             const fastjet::JetDefinition& jdef,
             JetAlg::MuonsStrategy usemuons=JetAlg::ALL_MUONS,
             JetAlg::InvisiblesStrategy useinvis=JetAlg::NO_INVISIBLES,
             fastjet::AreaDefinition* adef=nullptr)
      : JetAlg(fsp, usemuons, useinvis), _jdef(jdef), _adef(adef)
    {
      _initBase();
    }

    /// JetDefinition-based constructor with reordered args for easier specification of jet area definition
    ///
    /// @warning The AreaDefinition pointer must be heap-allocated: it will be stored/deleted via a shared_ptr.
    FastTopJets(const FinalState& fsp,
             const fastjet::JetDefinition& jdef,
             fastjet::AreaDefinition* adef,
             JetAlg::MuonsStrategy usemuons=JetAlg::ALL_MUONS,
             JetAlg::InvisiblesStrategy useinvis=JetAlg::NO_INVISIBLES)
      : FastTopJets(fsp, jdef, usemuons, useinvis, adef)
    {    }

    /// Native argument constructor, using FastJet alg/scheme enums.
    ///
    /// @warning The AreaDefinition pointer must be heap-allocated: it will be stored/deleted via a shared_ptr.
    FastTopJets(const FinalState& fsp,
             fastjet::JetAlgorithm type,
             fastjet::RecombinationScheme recom, double rparameter,
             JetAlg::MuonsStrategy usemuons=JetAlg::ALL_MUONS,
             JetAlg::InvisiblesStrategy useinvis=JetAlg::NO_INVISIBLES,
             fastjet::AreaDefinition* adef=nullptr)
      : FastTopJets(fsp, fastjet::JetDefinition(type, rparameter, recom), usemuons, useinvis, adef)
    {    }

    /// Native argument constructor with reordered args for easier specification of jet area definition
    ///
    /// @warning The AreaDefinition pointer must be heap-allocated: it will be stored/deleted via a shared_ptr.
    FastTopJets(const FinalState& fsp,
             fastjet::JetAlgorithm type,
             fastjet::RecombinationScheme recom, double rparameter,
             fastjet::AreaDefinition* adef,
             JetAlg::MuonsStrategy usemuons=JetAlg::ALL_MUONS,
             JetAlg::InvisiblesStrategy useinvis=JetAlg::NO_INVISIBLES)
      : FastTopJets(fsp, type, recom, rparameter, usemuons, useinvis, adef)
    {    }

    /// @brief Explicitly pass in an externally-constructed plugin
    ///
    /// @warning Provided plugin and area definition pointers must be heap-allocated; Rivet will store/delete via a shared_ptr
    FastTopJets(const FinalState& fsp,
             fastjet::JetDefinition::Plugin* plugin,
             JetAlg::MuonsStrategy usemuons=JetAlg::ALL_MUONS,
             JetAlg::InvisiblesStrategy useinvis=JetAlg::NO_INVISIBLES,
             fastjet::AreaDefinition* adef=nullptr)
      : FastTopJets(fsp, fastjet::JetDefinition(plugin), usemuons, useinvis, adef)
    {
      _plugin.reset(plugin);
    }

    /// @brief Explicitly pass in an externally-constructed plugin, with reordered args for easier specification of jet area definition
    ///
    /// @warning Provided plugin and area definition pointers must be heap-allocated; Rivet will store/delete via a shared_ptr
    FastTopJets(const FinalState& fsp,
             fastjet::JetDefinition::Plugin* plugin,
             fastjet::AreaDefinition* adef,
             JetAlg::MuonsStrategy usemuons=JetAlg::ALL_MUONS,
             JetAlg::InvisiblesStrategy useinvis=JetAlg::NO_INVISIBLES)
      : FastTopJets(fsp, plugin, usemuons, useinvis, adef)
    {    }

    /// @brief Convenience constructor using Rivet enums for most common jet algs (including some plugins).
    ///
    /// For the built-in algs, E-scheme recombination is used. For full control
    /// of FastJet built-in jet algs, use the constructors from native-args or a
    /// plugin pointer.
    ///
    /// @warning Provided area definition pointer must be heap-allocated; Rivet will store/delete via a shared_ptr
    FastTopJets(const FinalState& fsp,
             JetAlgName alg, double rparameter,
             JetAlg::MuonsStrategy usemuons=JetAlg::ALL_MUONS,
             JetAlg::InvisiblesStrategy useinvis=JetAlg::NO_INVISIBLES,
             fastjet::AreaDefinition* adef=nullptr,
             double seed_threshold=1.0)
      : JetAlg(fsp, usemuons, useinvis)
    {
      _initBase();
      _initJdef(alg, rparameter, seed_threshold);
    }


    // /// Same thing as above, but without an FS (for when we want to pass the particles directly to the calc method)
    // /// @todo Does this work properly, without internal HeavyQuarks etc.?
    // FastTopJets(JetAlgName alg, double rparameter, double seed_threshold=1.0) { _initJdef(alg, rparameter, seed_threshold); }
    // /// Same thing as above, but without an FS (for when we want to pass the particles directly to the calc method)
    // /// @todo Does this work properly, without internal HeavyQuarks etc.?
    // FastTopJets(fastjet::JetAlgorithm type, fastjet::RecombinationScheme recom, double rparameter) { _initJdef(type, recom, rparameter); }
    // /// Same thing as above, but without an FS (for when we want to pass the particles directly to the calc method)
    // /// @todo Does this work properly, without internal HeavyQuarks etc.?
    // FastTopJets(fastjet::JetDefinition::Plugin* plugin) : _jdef(plugin), _plugin(plugin) {
    //   // _plugin.reset(plugin);
    //   // _jdef = fastjet::JetDefinition(plugin);
    // }


    /// Clone on the heap.
    DEFAULT_RIVET_PROJ_CLONE(FastTopJets);

    //@}


    /// Reset the projection. Jet def, etc. are unchanged.
    void reset();


    /// @brief Use provided jet area definition
    ///
    /// @warning The provided pointer must be heap-allocated: it will be stored/deleted via a shared_ptr.
    /// @note Provide an adef null pointer to re-disable jet area calculation
    void useJetArea(fastjet::AreaDefinition* adef) {
      _adef.reset(adef);
    }


    /// @name Access to the jets
    //@{

    /// Get the jets (unordered) with pT > ptmin.
    Jets _jets() const;

    /// Get the pseudo jets (unordered).
    PseudoJets pseudoJets(double ptmin=0.0) const;
    /// Alias
    PseudoJets pseudojets(double ptmin=0.0) const { return pseudoJets(ptmin); }

    /// Get the pseudo jets, ordered by \f$ p_T \f$.
    PseudoJets pseudoJetsByPt(double ptmin=0.0) const {
      return sorted_by_pt(pseudoJets(ptmin));
    }
    /// Alias
    PseudoJets pseudojetsByPt(double ptmin=0.0) const { return pseudoJetsByPt(ptmin); }

    /// Get the pseudo jets, ordered by \f$ E \f$.
    PseudoJets pseudoJetsByE(double ptmin=0.0) const {
      return sorted_by_E(pseudoJets(ptmin));
    }
    /// Alias
    PseudoJets pseudojetsByE(double ptmin=0.0) const { return pseudoJetsByE(ptmin); }

    /// Get the pseudo jets, ordered by rapidity.
    PseudoJets pseudoJetsByRapidity(double ptmin=0.0) const {
      return sorted_by_rapidity(pseudoJets(ptmin));
    }
    /// Alias
    PseudoJets pseudojetsByRapidity(double ptmin=0.0) const { return pseudoJetsByRapidity(ptmin); }

    /// Trim (filter) a jet, keeping tag and constituent info in the resulting jet
    Jet trimJet(const Jet& input, const fastjet::Filter& trimmer) const;

    //@}


    /// @name Access to the FastJet clustering objects such as jet def, area def, and cluster
    //@{

    /// Return the cluster sequence.
    /// @todo Care needed re. const shared_ptr<T> vs. shared_ptr<const T>
    const shared_ptr<fastjet::ClusterSequence> clusterSeq() const {
      return _cseq;
    }
    // const fastjet::ClusterSequence* clusterSeq() const {
    //   return _cseq.get();
    // }

    /// Return the area-enabled cluster sequence (if an area defn exists, otherwise returns a null ptr).
    /// @todo Care needed re. const shared_ptr<T> vs. shared_ptr<const T>
    const shared_ptr<fastjet::ClusterSequenceArea> clusterSeqArea() const {
      return areaDef() ? dynamic_pointer_cast<fastjet::ClusterSequenceArea>(_cseq) : nullptr;
    }
    // const fastjet::ClusterSequenceArea* clusterSeqArea() const {
    //   return areaDef() ? dynamic_cast<fastjet::ClusterSequenceArea*>(_cseq.get()) : nullptr;
    // }

    /// Return the jet definition.
    const fastjet::JetDefinition& jetDef() const {
      return _jdef;
    }

    /// @brief Return the area definition.
    ///
    /// @warning May be null!
    /// @todo Care needed re. const shared_ptr<T> vs. shared_ptr<const T>
    const shared_ptr<fastjet::AreaDefinition> areaDef() const {
      return _adef;
    }
    // const fastjet::AreaDefinition* areaDef() const {
    //   return _adef.get();
    // }

    //@}


  private:

    /// Shared utility functions to implement constructor behaviour
    /// @todo Replace with calls between constructors when C++11 available?
    void _initBase();
    void _initJdef(JetAlgName alg, double rparameter, double seed_threshold);
    // void _init2(fastjet::JetAlgorithm type, fastjet::RecombinationScheme recom, double rparameter);
    // void _init3(const fastjet::JetDefinition& plugin);
    // void _init4(fastjet::JetDefinition::Plugin* plugin);

    /// Function to make Rivet::Jet from fastjet::PseudoJet, including constituent and tag info
    Jet _mkJet(const PseudoJet& pj) const;

  protected:

    /// Perform the projection on the Event.
    void project(const Event& e);
    
    /// agrohsje added for higgs and top tagging  
    void projectwtag(const Event& e, const Particles& tp);
    
    /// Compare projections.
    int compare(const Projection& p) const;

  public:

    /// Do the calculation locally (no caching).
    void calc(const Particles& fsparticles, const Particles& tagparticles=Particles());


  private:

    /// Jet definition
    fastjet::JetDefinition _jdef;

    /// Pointer to user-handled area definition
    std::shared_ptr<fastjet::AreaDefinition> _adef;

    /// Cluster sequence
    std::shared_ptr<fastjet::ClusterSequence> _cseq;

    /// FastJet external plugin
    std::shared_ptr<fastjet::JetDefinition::Plugin> _plugin;

    /// Map of vectors of y scales. This is mutable so we can use caching/lazy evaluation.
    mutable std::map<int, vector<double> > _yscales;

    /// set of particles sorted by their PT2
    //set<Particle, ParticleBase::byPTAscending> _particles;
    std::map<int, Particle> _particles;

  };

}

#endif
