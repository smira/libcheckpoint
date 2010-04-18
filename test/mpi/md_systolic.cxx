// md_systolic.cpp 
// Molecular Dynamics simulation of Lennard-Jones system
// Systolic algorithm using MPI

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#include "../../src/checkpoint.h"
#include "../../automatic/online/online.h"

// input parameters

int numberOfParticles = 32;
double density = 1.05;
double desiredTemperature = 1.0;

double timeStep = 0.004;
double equilibrationTime = 10.0;
int stepsBetweenRescalings = 10;
double simulationTime = 20.0;

// constants and variables

double box[3];			// length width height of periodic volume

const int MAX_PARTICLES = 5000;
double r[MAX_PARTICLES][3];	// positions
double v[MAX_PARTICLES][3];	// velocities
double a[MAX_PARTICLES][3];	// accelerations

double rTraveling[MAX_PARTICLES][3];

double t, temperature;
double kineticEnergy, potentialEnergy, totalEnergy, pressure, virial;

int measurementStep;
double temperatureSum, temperatureSqdSum;
double pressureSum, pressureSqdSum;
double potentialEnergySum, potentialEnergySqdSum;

int step;			// current integration time step
double wallClockTime, communicationTime;

// functions
void computeForces();
void initialize();
void takeOneTimeStep();
void initializeMeasurements();
void measureProperties();
int particleCount ();
void printProperties();
void rescaleVelocities();
void saveConfiguration (const char* fileName);
double gaussianDeviate();

// systolic algorithm variables
int numberOfProcesses;
int rankOfThisProcess;
int particlesPerProcess;
double sendBuffer[MAX_PARTICLES][3];

extern "C"
{
void ckptBenchmarkPrint();

int main (int argc, char* argv[]) 
{
    int i;
    double intPart[1], wallClockStartTime;
    char buf[256];

    wallClockTime = communicationTime = 0;
    MPI_Init(&argc, &argv);
    wallClockStartTime = MPI_Wtime();


    if (argc > 1)
	sscanf(argv[1], "%d", &numberOfParticles);
    if (argc > 2)
	sscanf(argv[2], "%lf", &density);
    if (argc > 3)
	sscanf(argv[3], "%lf", &desiredTemperature);
    if (argc > 2)
	sscanf(argv[2], "%lf", &timeStep);

    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankOfThisProcess);

        sprintf(buf, "mdsystolic.%d.log", rankOfThisProcess);
        freopen(buf, "w", stderr);
        setbuf(stderr, NULL);

    particlesPerProcess = numberOfParticles / numberOfProcesses;
    if (particlesPerProcess * numberOfProcesses != numberOfParticles) {
	if (rankOfThisProcess == 0) {
	    printf("Number of processes (%d) not an integral factor "
		   "of number of particles (%d)\nExiting ...\n",
		   numberOfProcesses, numberOfParticles);
	}
	MPI_Finalize();
	return 1;
    }
    
    if (rankOfThisProcess == 0) {
	printf("MD simulation of Lennard Jones System\n");
	printf("-------------------------------------\n");
	printf("Number of particles   = %d\n", numberOfParticles);
	printf("Particle density      = %f\n", density);
	printf("Desired Temperature   = %f\n", desiredTemperature);
	printf("Integration step size = %f\n", timeStep);
	printf("Number of Processes   = %d\n", numberOfProcesses);
    }
    initialize();
    saveConfiguration("initial.conf");
    if (rankOfThisProcess == 0) {
	printf("Cubical system volume = %f\n", box[0] * box[1] * box[2]);
	printf("Initial configuration written in file: initial.conf\n");
    }

    computeForces();
    t = 0;
    step = 0;
    if (rankOfThisProcess == 0) 
	printf("\nEquilibration steps, rescaling velocities every %d steps ...",
	       stepsBetweenRescalings);
    i = 0;
    initializeMeasurements();
    measureProperties();
    if (rankOfThisProcess == 0) 
	printf("\nTime = %.3f\tEnergy = %f", t, totalEnergy);
    while (t < equilibrationTime) {
	takeOneTimeStep();
	measureProperties();
	if (modf(t, intPart) < timeStep)
	    if (rankOfThisProcess == 0) 
		printf("\nTime = %.3f\tEnergy = %f", intPart[0], totalEnergy);
	if (++i >= stepsBetweenRescalings) {
	    rescaleVelocities();
	    i = 0;
	}
    }
    if (rankOfThisProcess == 0) 
	printf("\nProduction steps ...");
    initializeMeasurements();
    while (t < equilibrationTime + simulationTime) {
	takeOneTimeStep();
	measureProperties();
	if (modf(t, intPart) < timeStep)
	{
	    if (rankOfThisProcess == 0) 
	    {
		printf("\nTime = %.3f\tEnergy = %f", intPart[0], totalEnergy);
		ckptMPIOnlineCheckpoint();
	    }
	}
    }
    if (rankOfThisProcess == 0) {
	printf("\nCompleted %d time steps ...\n", step);
	printProperties();
	printf("Final configuration written in file: final.conf\n");
    }
    saveConfiguration("final.conf");
    wallClockTime = MPI_Wtime() - wallClockStartTime;
    if (rankOfThisProcess == 0) {
	printf("Interprocess communication time = %f sec\n", communicationTime);
	printf("Total elapsed wall clock time   = %f sec\n", wallClockTime);
    }

    MPI_Finalize();
    ckptBenchmarkPrint();
}
}

void forceAndPotentialEnergy (double ri[], double rj[],
			      double ai[], double aj[],
			      double *pairPotentialEnergy) {
    int k, sign;
    double rSqd, rij[3], mag;
    double rInv2, rInv6, rInv12, aij[3];
    
    rSqd = 0.0;
    for (k = 0; k < 3; k++) {
	rij[k] = ri[k] - rj[k];
	sign = rij[k] > 0.0 ? 1 : -1;
	mag = rij[k] * sign;
	if (mag > box[k] / 2)            // use nearest image
	    rij[k] -= box[k] * sign;
	rSqd += rij[k] * rij[k];
    }
    rInv2 = 1 / rSqd;
    rInv6 = rInv2 * rInv2 * rInv2;
    rInv12 = rInv6 * rInv6;
    mag = rInv2 * (48 * rInv12 - 24 * rInv6);
    for (k = 0; k < 3; k++) {
	aij[k] = rij[k] * mag;
	ai[k] += aij[k];
	aj[k] -= aij[k];
    }
    *pairPotentialEnergy = 4 * (rInv12 - rInv6);
}

void computeForces () {
    int i, j, k, process, nextProcess, previousProcess;
    double potentialEnergyThisProcess, pairPotentialEnergy, aJunk[3];
    double startTime;
    MPI_Status status;

    for (i = 0; i < particlesPerProcess; i++)
	for (k = 0; k < 3; k++)
	    a[i][k] = 0.0;
    potentialEnergy = potentialEnergyThisProcess = 0.0;

    // loop over all pairs of local particles
    for (i = 0; i < particlesPerProcess - 1; i++) {
	for (j = i + 1; j < particlesPerProcess; j++) {
	    forceAndPotentialEnergy(r[i], r[j], a[i], a[j],
				    &pairPotentialEnergy);
	    potentialEnergyThisProcess += pairPotentialEnergy;
	}
    }

    // copy positions of local particles to traveling particles
    for (i = 0; i < particlesPerProcess; i++)
	for (k = 0; k < 3; k++)
	    rTraveling[i][k] = r[i][k];

    // implement systolic algorithm
    for (process = 0; process < numberOfProcesses - 1; process++) {
	startTime = MPI_Wtime();
	// copy traveling particles to send buffer
	for (i = 0; i < particlesPerProcess; i++)
	    for (k = 0; k < 3; k++)
		sendBuffer[i][k] = rTraveling[i][k];
	// next and previous processes in ring topology
	nextProcess = rankOfThisProcess + 1;
	if (nextProcess == numberOfProcesses)
	    nextProcess = 0;
	previousProcess = rankOfThisProcess - 1;
	if (previousProcess == -1)
	    previousProcess = numberOfProcesses - 1;
	// interleave send/receive to prevent deadlock
	if (rankOfThisProcess % 2 == 0) {  // even processes send/receive
	    // send traveling particles to next process
	    MPI_Send(sendBuffer[0], 3 * particlesPerProcess, MPI_DOUBLE,
		     nextProcess, 0, MPI_COMM_WORLD);
	    // receive traveling particles from previous process
	    MPI_Recv(rTraveling[0], 3 * particlesPerProcess, MPI_DOUBLE,
		     previousProcess, 0, MPI_COMM_WORLD, &status); 
	} else {                           // odd processes receive/send
	    MPI_Recv(rTraveling[0], 3 * particlesPerProcess, MPI_DOUBLE,
		     previousProcess, 0, MPI_COMM_WORLD, &status); 
	    MPI_Send(sendBuffer[0], 3 * particlesPerProcess, MPI_DOUBLE,
		     nextProcess, 0, MPI_COMM_WORLD);
	}
	communicationTime += MPI_Wtime() - startTime;
	// calculate contributions to local forces from traveling particles
	for (i = 0; i < particlesPerProcess; i++) {
	    for (j = 0; j < particlesPerProcess; j++) {
		forceAndPotentialEnergy(r[i], rTraveling[j], a[i], aJunk,
					&pairPotentialEnergy);
		potentialEnergyThisProcess += 0.5 * pairPotentialEnergy;
	    }
	}
    }
    // accumulate total potential energy
    startTime = MPI_Wtime();
    MPI_Allreduce(&potentialEnergyThisProcess, &potentialEnergy,
		  1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    communicationTime += MPI_Wtime() - startTime;
}

void initialize () {
    int c, i, j, k, m, n, p;
    double L, b, vThisProcessSum[3], vSum[3];
    double rFCC[4][3] = {{0.0, 0.0, 0.0}, {0.0, 0.5, 0.5},
			 {0.5, 0.0, 0.5}, {0.5, 0.5, 0.0}};
    double rCell[3];
    double startTime;

    // compute length width height of box
    L = pow(numberOfParticles / density, 1.0 / 3.0);
    for (i = 0; i < 3; i++)
	box[i] = L;

    // use face centered cubic (fcc) lattice for initial positions
    // find number c of unit cells needed to place all particles
    for (c = 1; ; c++)
	if (4*c*c*c >= numberOfParticles)
	    break;
    b = L / c;			// side of conventional unit cell
    p = 0;			// particles placed so far
    for (i = 0; i < c; i++) {
	rCell[0] = i * b;
	for (j = 0; j < c; j++) {
	    rCell[1] = j * b;
	    for (k = 0; k < c; k++) {
		rCell[2] = k * b;
		for (m = 0; m < 4; m++)	// 4 particles in cell
		    if (p < numberOfParticles) {
			for (n = 0; n < 3; n++)  // x, y, z
			    r[p][n] = rCell[n] + b * rFCC[m][n];
			++p;
		    }
	    }
	}
    }

    // distribute initial positions among processes
    if (rankOfThisProcess > 0) {
	p = rankOfThisProcess * particlesPerProcess;
	for (n = 0; n < particlesPerProcess; n++)
	    for (i = 0; i < 3; i++)
		r[n][i] = r[p + n][i];
	++p;
    }

    // initial velocities
    for (i = 0; i < 3; i++)
	vThisProcessSum[i] = vSum[i] = 0;
    // initialize the random number generator for this process
    for (i = 0; i < rankOfThisProcess * particlesPerProcess * 3; i++)
	gaussianDeviate();
    // random Gaussian distributed initial velocities
    for (p = 0; p < particlesPerProcess; p++) {
	for (i = 0; i < 3; i++) {
	    v[p][i] = gaussianDeviate();
	    vThisProcessSum[i] += v[p][i];
	}
    }
    // compute total momentum
    startTime = MPI_Wtime();
    MPI_Allreduce(vThisProcessSum, vSum, 3, MPI_DOUBLE,
		  MPI_SUM, MPI_COMM_WORLD);
    communicationTime += MPI_Wtime() - startTime;
    // zero total momentum
    for (p = 0; p < particlesPerProcess; p++)
	for (i = 0; i < 3; i++)
	    v[p][i] -= vSum[i] / numberOfParticles;
    // rescaled to desired temperature
    rescaleVelocities();
}

void takeOneTimeStep () {
    int p, k;

    for (p = 0; p < particlesPerProcess; p++)
	for (k = 0; k < 3; k++) {
	    r[p][k] += v[p][k] * timeStep
		+ 0.5 * a[p][k] * timeStep * timeStep;
	    // impose periodic boundary conditions
	    if (r[p][k] < 0.0)
		r[p][k] += box[k];
	    if (r[p][k] >= box[k])
		r[p][k] -= box[k];
	    v[p][k] += 0.5 * a[p][k] * timeStep;
	}
    computeForces();
    for (p = 0; p < particlesPerProcess; p++)
	for (k = 0; k < 3; k++)
	    v[p][k] += 0.5 * a[p][k] * timeStep;

    t += timeStep;
    ++step;
}

void measureProperties () {
    int p, i;
    double kineticEnergyThisProcess, virialThisProcess;
    double startTime;
    
    kineticEnergy = virial = 0;
    kineticEnergyThisProcess = virialThisProcess = 0;
    for (p = 0; p < particlesPerProcess; p++) {
	for (i = 0; i < 3; i++) {
	    kineticEnergyThisProcess += 0.5 * v[p][i] * v[p][i];
	    virialThisProcess += r[p][i] * a[p][i];
	}
    }
    // accumulate total kinetic energy and virial
    startTime = MPI_Wtime();
    MPI_Allreduce(&kineticEnergyThisProcess, &kineticEnergy,
		  1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&virialThisProcess, &virial,
		  1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    communicationTime += MPI_Wtime() - startTime;

    totalEnergy = kineticEnergy + potentialEnergy;
    temperature = 2.0 * kineticEnergy / 3.0 / numberOfParticles;
    pressure = temperature + virial / 3.0 / numberOfParticles;
    pressure *= density;

    ++measurementStep;
    temperatureSum += temperature;
    temperatureSqdSum += temperature * temperature;
    pressureSum += pressure;
    pressureSqdSum += pressure * pressure;
    potentialEnergySum += potentialEnergy;
    potentialEnergySqdSum += potentialEnergy * potentialEnergy;
}

void rescaleVelocities () {
    int i, p;
    double vSqdSum, vSqdSumThisProcess, scale;
    double startTime;

    vSqdSum = vSqdSumThisProcess = 0;
    for (p = 0; p < particlesPerProcess; p++)
	for (i = 0; i < 3; i++)
	    vSqdSumThisProcess += v[p][i] * v[p][i];
    startTime = MPI_Wtime();
    MPI_Allreduce(&vSqdSumThisProcess, &vSqdSum,
		  1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    communicationTime += MPI_Wtime() - startTime;
    scale = 3 * (numberOfParticles) * desiredTemperature / vSqdSum;
    scale = sqrt(scale);
    for (p = 0; p < particlesPerProcess; p++)
	for (i = 0; i < 3; i++)
	    v[p][i] *= scale;
}

void initializeMeasurements () {
    measurementStep = 0;
    temperatureSum = temperatureSqdSum = 0;
    pressureSum = pressureSqdSum = 0;
    potentialEnergySum = potentialEnergySqdSum = 0;
}

int particleCount () {
    int d, n, nInBoxThisProcess, nInBox, inBox;
    double startTime;
     
    nInBox = nInBoxThisProcess = 0;
    for (n = 0; n < particlesPerProcess; n++) {
	inBox = 1;
	for (d = 0; d < 3; d++)
	    if (r[n][d] < 0.0 || r[n][d] > box[d])
		inBox = 0;
	if (inBox)
	    ++nInBox;
    }
    startTime = MPI_Wtime();
    MPI_Allreduce(&nInBoxThisProcess, &nInBox, 1, MPI_INT,
		  MPI_SUM, MPI_COMM_WORLD);
    communicationTime += MPI_Wtime() - startTime;
    return nInBox;
}

void printProperties () {
    double average, stdDev;

    average = temperatureSum / measurementStep;
    stdDev = temperatureSqdSum / measurementStep;
    stdDev = sqrt(stdDev - average * average);
    printf("Temperature = %f  +-  %f\n", average, stdDev);
    
    average = pressureSum / measurementStep;
    stdDev = pressureSqdSum / measurementStep;
    stdDev = sqrt(stdDev - average * average);
    printf("Pressure = %f  +-  %f\n", average, stdDev);
    
    average = potentialEnergySum / measurementStep;
    stdDev = potentialEnergySqdSum / measurementStep;
    stdDev = sqrt(stdDev - average * average);
    printf("Potential Energy per particle = %f  +-  %f\n",
	   average / numberOfParticles, stdDev / numberOfParticles);
}

double gaussianDeviate () {
    static int available = 0;
    static double savedDeviate;
    double r[2], rSqd, factor;
    int i;

    if (available) {
	available = 0;
	return savedDeviate;
    }
    do {
	rSqd = 0.0;
	for (i = 0; i < 2; i++) {
	    r[i] = (2.0 * (double) rand()) / RAND_MAX - 1.0;
	    rSqd += r[i] * r[i];
	}
    } while (rSqd >= 1.0 || rSqd == 0.0);
    factor = sqrt(-2.0 * log(rSqd) / rSqd);
    savedDeviate = r[0] * factor;
    available = 1;
    return r[1] * factor;
}

void saveConfiguration (const char* fileName) {
    FILE *dataFile;
    int i, k, p;
    MPI_Status status;
    double startTime;

    // copy particle positions to process 0 from other processes
    startTime = MPI_Wtime();
    if (rankOfThisProcess == 0) {
	for (p = 1; p < numberOfProcesses; p++)
	    MPI_Recv(r[p * particlesPerProcess],
		     3 * particlesPerProcess, MPI_DOUBLE,
		     p, 0, MPI_COMM_WORLD, &status);
    } else {
	for (i = 0; i < particlesPerProcess; i++)
	    for (k = 0; k < 3; k++)
		sendBuffer[i][k] = r[i][k];
	MPI_Send(sendBuffer[0], 3 * particlesPerProcess, MPI_DOUBLE,
		 0, 0, MPI_COMM_WORLD);
	return;
    }
    communicationTime += MPI_Wtime() - startTime;

    dataFile = fopen(fileName, "w");
    fprintf(dataFile, "%d\n", numberOfParticles);
    for (i = 0; i < 3; i++)
	fprintf(dataFile, "%f\t%f\n", 0.0, box[i]);
    for (i = 0; i < numberOfParticles; i++)
	fprintf(dataFile, "%f\t%f\t%f\n", r[i][0], r[i][1], r[i][2]);
    fclose(dataFile);
}
