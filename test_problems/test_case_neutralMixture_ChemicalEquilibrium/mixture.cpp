#include <time.h>
#include <iostream>
#include <vector>
#include <cantera/Cantera.h>
#include <cantera/thermo/ThermoPhase.h>			
#include <cantera/thermo/ThermoFactory.h>		
#include <cantera/equilibrium.h>
#include <fstream>
#include <iomanip>
#include <cantera/transport/TransportFactory.h>	
#include <string>


using namespace Cantera;
using namespace std;
void equil_demo() {


	ofstream f("speciesName.txt");
	ofstream fd("qReactive_Mixture.txt");
        ofstream fe2("electronTranslationalThCond.txt");
        ofstream fe3("translationalThCond_Mixture.txt");
        ofstream ffGY("viscosity_MixtureGY.txt");

        ofstream qj("speciesDiffusiveMassFluxes_GMRES.txt");
        ofstream qj2("speciesDiffusiveMassFluxesSM_GMRES.txt");
        ofstream qj3("speciesDiffusiveMassFluxesSM_CG_Eamb.txt");

        ofstream f10("molarFractions_VS_Temperature.txt");
        ofstream fen("enthalpy_mass.txt");

					
	double T_in = 300;		// Initial Temperature (K)
	double T_fin = 30000;		// Final Temperature (K)
	double T=T_in;           	// Initialization of Temperature
	double delta = 100;		// delta_T: increment for Temperature
	double eps=0.0001;       	// epsilon (for finite difference computation)
        double epsP=1+eps;      	// epsilonP (for finite difference computation)
	double T2=epsP*T;               // Initialize temperature T2 (for gradient computation)
	int a = 20;			// offset for external document


        ThermoPhase* gas = newPhase("gri_CO25.xml","gri30_mix");     // Generate new phase
        ThermoPhase* gas2 = newPhase("gri_CO25.xml","gri30_mix");    // Generate new phase for grad_X calculation


	Transport* tr = newTransportMgr("Mix", gas);		// Transport manager for gas (transport properties calculation)
	int	numSpecies = gas->nSpecies();		// Number of species in the mixture
	string	nameSpecies[numSpecies];		// to define the vector for Name of species


	double weights[numSpecies];			// to create a vector (called weights) for the molecular weights of each species
        gas->getMolecularWeights(weights);		// to save in weights the molecular weights

        double MolarEnthalpies[numSpecies];		// to create a vector (called MolarEnthalpies) for the molar enthalpies of each species

	double JDiff[numSpecies];			// to create a vector (called JDiff) for the species diffusive mass fluxes [kg/m2/s]
        double JDiff_SM[numSpecies];                    // to create a vector (called JDiff) for the species diffusive mass fluxes [kg/m2/s]
	double JDiff_Eamb[numSpecies];

	double grad_T[1]={1};				// to create a vector (N.B. nDim=1)  for Temperature Gradient (N.B. isotropy: no preferential directions)	

        double grad_X[numSpecies];			// to create a vector for Gradients of the mole fraction
        for (int b=0; b<numSpecies; b++)		// Initialize grad_T and grad_X
        {grad_X[b]=1;}

	double qReactiveSpecies[numSpecies];		// to create a vector for the species qReactive (qReactive_k = Jdiff_k * MassEnthalpy_k)
	double qReactive=0;             		// initialize qReactive (qReactive = Sum_k qReactive_k)

	double mixAverDiffCoeff_mass[numSpecies];


	f10 << setw(a) << left << "T[K]";

	for (int b1=0; b1<numSpecies; b1++)
        {
		nameSpecies[b1] = gas->speciesName(b1); 					// to save in nameSpecies the name of each species
                f   << setw(a) << left << "The species named " << setw(a-10) << left << b1 << setw(a-10) << left << " is: " << setw(a-10) << left << nameSpecies[b1];
		f   << setw(a-10) << left << "molecular weight: " << setw(a) << left << weights[b1] << endl;

                f10 << setw(a) << left << nameSpecies[b1];

	}
        f10 << endl;


for (int T=T_in; T<T_fin; T = T + delta)
{

		T2=epsP*T;       	// Set T2 for computations
		qReactive=0;            // reset qReactive for computations

                gas->setState_TPX(T, 1.0*OneAtm,"C:0.333, O:0.667");      // Set the state for the Mixture
                gas2->setState_TPX(T2, 1.0*OneAtm,"C:0.333, O:0.667");    // Set the state for the Mixture gas2 (for grad_X calculation)


		equilibrate(*gas, "TP");                                                 	// Equilibrium
		equilibrate(*gas2, "TP");						 	// Equilibrium

		tr->getMixDiffCoeffsMass(mixAverDiffCoeff_mass);


		gas->getPartialMolarEnthalpies(MolarEnthalpies);        			// to save in MolarEnthalpies the molar enthalpies

		// Save in grad_X the Mole Fraction Gradients corresponding to a unitary Temperature Gradiend (P.S. constant pressure)
		for(int c=0; c<numSpecies; c++)
                {
                	grad_X[c]=(gas2->moleFraction(c) - gas->moleFraction(c))/(T2-T);
              	}


         	// Save in the vector JDiff the species diffusive mass fluxes [kg/m2/s]
             	tr->getSpeciesFluxes(1, grad_T, numSpecies, grad_X, numSpecies, JDiff);
		tr->getSpeciesFluxesNeutSM(1, grad_T, numSpecies, grad_X, numSpecies, JDiff_SM);


                qj << setw(a) << left << T;
                qj2 << setw(a) << left << T;
                qj3 << setw(a) << left << T;


		// Compute the qReactive [J/m2/s] for each species and the total qReactive (N.B. since grad_T=1, qReactive coincide with the reactive term of thermal conductivity)
           	for(int c1=0; c1<numSpecies; c1++)
               	{
              		qReactiveSpecies[c1] = JDiff_SM[c1]*MolarEnthalpies[c1]/weights[c1];
			qReactive = qReactive - qReactiveSpecies[c1];

			qj << setw(a) << left << JDiff[c1];
                        qj2 << setw(a) << left << JDiff_SM[c1];
                        qj3 << setw(a) << left << JDiff_Eamb[c1];


		}

		qj << endl;
                qj2 << endl;
                qj3 << endl;


                f10 << setw(a) << left << T;
		for (int c2=0; c2<numSpecies; c2++)
                	{
                        	f10 << setw(a) << left << gas->moleFraction(c2);
			}
                f10 << endl;

		// For mixture: to write the external files
		fd  << setw(a) << left << T << qReactive << endl;
                fe3  << setw(a) << left << T << tr->translationalThermalConductivity() << endl;
                ffGY  << setw(a) << left << T << tr->viscosityGY() << endl;
		fen << setw(a) << left << T << setw(a) << left << gas->enthalpy_mass() << endl;

}


	delete gas;
	delete gas2;

	f.close();
	fd.close();
        fe2.close();
        fe3.close();
        ffGY.close();
        f10.close();
	fen.close();
	qj.close();
        qj2.close();
        qj3.close();


}
 
int main() {


//original
    try {
	clock_t time;
        time = clock();
        equil_demo();
	time = clock() - time;
        cout << "--------------------------------" << endl << "Elapsed time: " << ((double)time)/CLOCKS_PER_SEC << endl << "--------------------------------" << endl;
	return 0;
    }


    catch (CanteraError) {
        showErrors(cout);
	cout << "terminating ... " << endl;
	appdelete();
	return 1;
    }
}
