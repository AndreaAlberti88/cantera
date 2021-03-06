/**
 *  @file StoichSubstanceSSTP.cpp
 * Definition file for the StoichSubstanceSSTP class, which represents a fixed-composition
 * incompressible substance (see \ref thermoprops and
 * class \link Cantera::StoichSubstanceSSTP StoichSubstanceSSTP\endlink)
 */

/*
 * Copyright (2005) Sandia Corporation. Under the terms of
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 *
 * Copyright 2001 California Institute of Technology
 */
#include "cantera/thermo/StoichSubstanceSSTP.h"
#include "cantera/thermo/mix_defs.h"
#include "cantera/thermo/ThermoFactory.h"
#include "cantera/base/ctml.h"

namespace Cantera
{

/*
 * ----  Constructors -------
 */

StoichSubstanceSSTP::StoichSubstanceSSTP(const std::string& infile, const std::string& id_)
{
    initThermoFile(infile, id_);
}

StoichSubstanceSSTP::StoichSubstanceSSTP(XML_Node& xmlphase, const std::string& id_)
{
    importPhase(xmlphase, this);
}

StoichSubstanceSSTP::StoichSubstanceSSTP(const StoichSubstanceSSTP& right)
{
    *this = right;
}

StoichSubstanceSSTP&
StoichSubstanceSSTP::operator=(const StoichSubstanceSSTP& right)
{
    if (&right != this) {
        SingleSpeciesTP::operator=(right);
    }
    return *this;
}

ThermoPhase* StoichSubstanceSSTP::duplMyselfAsThermoPhase() const
{
    return new StoichSubstanceSSTP(*this);
}

/*
 * ---- Utilities -----
 */

int StoichSubstanceSSTP::eosType() const
{
    return cStoichSubstance;
}

/*
 * ----- Mechanical Equation of State ------
 */

doublereal StoichSubstanceSSTP::pressure() const
{
    return m_press;
}

void StoichSubstanceSSTP::setPressure(doublereal p)
{
    m_press = p;
}

doublereal StoichSubstanceSSTP::isothermalCompressibility() const
{
    return 0.0;
}

doublereal StoichSubstanceSSTP::thermalExpansionCoeff() const
{
    return 0.0;
}

/*
 * ---- Chemical Potentials and Activities ----
 */

void StoichSubstanceSSTP::getActivityConcentrations(doublereal* c) const
{
    c[0] = 1.0;
}

doublereal StoichSubstanceSSTP::standardConcentration(size_t k) const
{
    return 1.0;
}

doublereal StoichSubstanceSSTP::logStandardConc(size_t k) const
{
    return 0.0;
}

/*
 * Properties of the Standard State of the Species in the Solution
 */

void StoichSubstanceSSTP::getStandardChemPotentials(doublereal* mu0) const
{
    getGibbs_RT(mu0);
    mu0[0] *= GasConstant * temperature();
}

void StoichSubstanceSSTP::getEnthalpy_RT(doublereal* hrt) const
{
    getEnthalpy_RT_ref(hrt);
    doublereal presCorrect = (m_press - m_p0) / molarDensity();
    hrt[0] += presCorrect / (GasConstant * temperature());
}

void StoichSubstanceSSTP::getEntropy_R(doublereal* sr) const
{
    getEntropy_R_ref(sr);
}

void StoichSubstanceSSTP::getGibbs_RT(doublereal* grt) const
{
    getEnthalpy_RT(grt);
    grt[0] -= m_s0_R[0];
}

void StoichSubstanceSSTP::getCp_R(doublereal* cpr) const
{
    _updateThermo();
    cpr[0] = m_cp0_R[0];
}

void StoichSubstanceSSTP::getIntEnergy_RT(doublereal* urt) const
{
    _updateThermo();
    urt[0] = m_h0_RT[0] - m_p0 / molarDensity() / (GasConstant * temperature());
}

/*
 * ---- Thermodynamic Values for the Species Reference States ----
 */

void StoichSubstanceSSTP::getIntEnergy_RT_ref(doublereal* urt) const
{
    _updateThermo();
    urt[0] = m_h0_RT[0] - m_p0 / molarDensity() / (GasConstant * temperature());
}

/*
 * ---- Initialization and Internal functions
 */

void StoichSubstanceSSTP::initThermo()
{
    /*
     * Make sure there is one and only one species in this phase.
     */
    if (m_kk != 1) {
        throw CanteraError("initThermo",
                           "stoichiometric substances may only contain one species.");
    }
    /*
     * Store the reference pressure in the variables for the class.
     */
    m_p0 = refPressure();

    /*
     * Resize temporary arrays.
     */
    int leng = 1;
    m_h0_RT.resize(leng);
    m_cp0_R.resize(leng);
    m_s0_R.resize(leng);
    /*
     * Call the base class thermo initializer
     */
    SingleSpeciesTP::initThermo();
}

void StoichSubstanceSSTP::initThermoXML(XML_Node& phaseNode, const std::string& id_)
{
    /*
     * Find the Thermo XML node
     */
    if (!phaseNode.hasChild("thermo")) {
        throw CanteraError("StoichSubstanceSSTP::initThermoXML",
                           "no thermo XML node");
    }
    XML_Node& tnode = phaseNode.child("thermo");
    std::string model = tnode["model"];
    if (model != "StoichSubstance" && model != "StoichSubstanceSSTP") {
        throw CanteraError("StoichSubstanceSSTP::initThermoXML",
                           "thermo model attribute must be StoichSubstance");
    }
    double dens = getFloatDefaultUnits(tnode, "density", "kg/m3");
    setDensity(dens);
    SingleSpeciesTP::initThermoXML(phaseNode, id_);
}

void StoichSubstanceSSTP::setParameters(int n, doublereal* const c)
{
    setDensity(c[0]);
}

void StoichSubstanceSSTP::getParameters(int& n, doublereal* const c) const
{
    n = 1;
    c[0] = density();
}

void StoichSubstanceSSTP::setParametersFromXML(const XML_Node& eosdata)
{
    std::string model = eosdata["model"];
    if (model != "StoichSubstance" && model != "StoichSubstanceSSTP") {
        throw CanteraError("StoichSubstanceSSTP::setParametersFromXML",
                           "thermo model attribute must be StoichSubstance");
    }
    setDensity(getFloat(eosdata, "density", "toSI"));
}

}
