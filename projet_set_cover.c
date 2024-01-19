#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define POPULATION_SIZE 80
#define MUTATION_PROB 0.2
#define INDIVIDUAL_MUTATION_PROB 0.3
#define MAX_GENERATIONS 100

typedef struct {
    int *chromosome;
    int fitness;
} Individual;

void readMatrixFromFile(const char *filename, Individual **problemMatrix, int *num_elements, int *num_sets) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier.");
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%d", num_elements) != 1 || fscanf(file, "%d", num_sets) != 1) {
        fprintf(stderr, "Erreur lors de la lecture des dimensions du fichier.");
        exit(EXIT_FAILURE);
    }

    *problemMatrix = malloc((*num_sets) * sizeof(Individual));
    if (*problemMatrix == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire.");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < *num_sets; i++) {
        (*problemMatrix)[i].chromosome = malloc((*num_elements) * sizeof(int));
        if ((*problemMatrix)[i].chromosome == NULL) {
            fprintf(stderr, "Erreur d'allocation mémoire.");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < *num_elements; j++) {
            if (fscanf(file, "%d", &((*problemMatrix)[i].chromosome[j])) != 1) {
                fprintf(stderr, "Erreur de lecture du fichier.");
                exit(EXIT_FAILURE);
            }
        }
        (*problemMatrix)[i].fitness = -1; // Initialise la fitness
    }

    fclose(file);
}


void printProblemMatrix(Individual *population, int num_sets, int num_elements) {
    for (int i = 0; i < num_sets; i++) {
        printf("Individu %d: Chromosome [ ", i + 1);
        for (int j = 0; j < num_elements; j++) {
            printf("%d ", population[i].chromosome[j]);
        }
        printf("] Fitness: %d\n", population[i].fitness);
    }
}

void initializePopulation(Individual population[], int num_elements) {
    for (int i = 0; i < POPULATION_SIZE; i++) {
        population[i].chromosome = malloc(num_elements * sizeof(int));
        if (population[i].chromosome == NULL) {
            fprintf(stderr, "Erreur d'allocation mémoire pour le chromosome.");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < num_elements; j++) {
            population[i].chromosome[j] = rand() % 2;
        }
        population[i].fitness = -1;
    }
}

void performCrossover(Individual *parent1, Individual *parent2, Individual *child1,
                        Individual *child2, int num_elements, int crossoverPoint) {
    for (int i = 0; i < crossoverPoint; i++) {
        child1->chromosome[i] = parent1->chromosome[i];
        child2->chromosome[i] = parent2->chromosome[i];
    }
    for (int i = crossoverPoint; i < num_elements; i++) {
        child1->chromosome[i] = parent2->chromosome[i];
        child2->chromosome[i] = parent1->chromosome[i];
    }
    child1->fitness = child2->fitness = -1;
}

int mutate(Individual *individual, int num_elements) {
    int mutated = 0;
    if ((double)rand() / RAND_MAX < MUTATION_PROB) {
        for (int j = 0; j < num_elements; j++) {
            if ((double)rand() / RAND_MAX < INDIVIDUAL_MUTATION_PROB) {
                individual->chromosome[j] = 1 - individual->chromosome[j];
                mutated = 1;
            }
        }
    }
    return mutated;
}

void freeChromosomes(Individual *population, int size) {
    for (int i = 0; i < size; i++) {
        free(population[i].chromosome);
    }
}

void evaluateFitness(Individual *individual, Individual *problemMatrix, int num_elements, int num_sets) {
    int covered[num_elements];
    memset(covered, 0, sizeof(covered));
    int coverCount = 0;
    int overCovered = 0;

    for (int i = 0; i < num_sets; i++) {
        if (individual->chromosome[i] == 1) {
            for (int j = 0; j < num_elements; j++) {
                if (problemMatrix[i].chromosome[j] == 1) {
                    covered[j]++;
                    if (covered[j] == 1) {
                        coverCount++;
                    } else {
                        overCovered = 1; // Marque si un élément est couvert plus d'une fois
                    }
                }
            }
        }
    }

    if (coverCount == num_elements && !overCovered) { // Tous les éléments sont couverts exactement une fois
        individual->fitness = 0;
        for (int i = 0; i < num_sets; i++) {
            if (individual->chromosome[i] == 1) {
                individual->fitness++;
            }
        }
    } else {
        individual->fitness = MAX_GENERATIONS; // Pénalité si la solution n'est pas valide
    }
}


int compareIndividuals(const void *a, const void *b) {
    Individual *indA = (Individual *)a;
    Individual *indB = (Individual *)b;
    return indA->fitness - indB->fitness;
}

void findSetCoverSolutions(Individual *population, Individual *problemMatrix, int num_elements, int num_sets) {
    for (int gen = 0; gen < MAX_GENERATIONS; gen++) {
        // Évaluation de la fitness pour chaque individu
        for (int i = 0; i < POPULATION_SIZE; i++) {
            evaluateFitness(&population[i], problemMatrix, num_elements, num_sets);
        }
    }

    // Trier la population par fitness
    qsort(population, POPULATION_SIZE, sizeof(Individual), compareIndividuals);


    // Collecter les solutions viables
    Individual solutions[POPULATION_SIZE];
    int solutionsCount = 0;

    for (int i = 0; i < POPULATION_SIZE; i++) {
        if (population[i].fitness < MAX_GENERATIONS) {
            solutions[solutionsCount++] = population[i];
        }
    }

    // Affichage des solutions
    printf("Nombre de solutions trouvees : %d\n", solutionsCount);
    for (int i = 0; i < solutionsCount; i++) {
        for (int j = 0; j < num_sets; j++) {
            printf("%d ", solutions[i].chromosome[j]);
        }
        printf("\n");
    }

    // Afficher à nouveau les solutions après le tri
    printf("Tableau de solution trie :\n");
    for (int i = 0; i < solutionsCount; i++) {
        for (int j = 0; j < num_sets; j++) {
            printf("%d ", solutions[i].chromosome[j]);
        }
        printf("\n");
    }
}

int createMutationPopulation(Individual population[], Individual mutation[], int num_elements) {
    int mutationCount = 0;
    for (int i = 0; i < POPULATION_SIZE; i++) {
        Individual temp;
        temp.chromosome = malloc(num_elements * sizeof(int));
        memcpy(temp.chromosome, population[i].chromosome, num_elements * sizeof(int));
        
        if (mutate(&temp, num_elements)) {
            mutation[mutationCount] = temp;
            mutation[mutationCount].fitness = -1;
            mutationCount++;
        } else {
            free(temp.chromosome);
        }
    }
    return mutationCount;
}

void createChildrenPopulation(Individual population[], Individual children[], int num_elements, int crossoverPoint) {
    for (int i = 0; i < POPULATION_SIZE; i += 2) {
        children[i].chromosome = malloc(num_elements * sizeof(int));
        children[i + 1].chromosome = malloc(num_elements * sizeof(int));

        Individual *parent1 = &population[i];
        Individual *parent2 = &population[(i + 1) % POPULATION_SIZE];
        performCrossover(parent1, parent2, &children[i], &children[i + 1], num_elements, crossoverPoint);
    }
}


int main() {
    srand(time(NULL));

    Individual *problemMatrix;
    int num_elements, num_sets;

    readMatrixFromFile("matrix.txt", &problemMatrix, &num_elements, &num_sets);
    

    int crossoverPoint = rand() % num_sets;
    printf("Le nombre aleatoire : %d\n",crossoverPoint);

    Individual population[POPULATION_SIZE];
    initializePopulation(population, num_sets);

    Individual children[POPULATION_SIZE];
    createChildrenPopulation(population, children, num_sets, crossoverPoint);

    Individual mutation[POPULATION_SIZE];
    int mutationCount = createMutationPopulation(population, mutation, num_sets);

    
    printf("Population probleme matrix :\n");
    printProblemMatrix(problemMatrix, num_sets, num_elements);
    printf("Population initiale:\n");
    printProblemMatrix(population, POPULATION_SIZE, num_sets);
    printf("Population d'enfants:\n");
    printProblemMatrix(children, POPULATION_SIZE, num_sets);
    printf("Population de mutation (total: %d):\n", mutationCount);
    printProblemMatrix(mutation, mutationCount, num_sets);

    findSetCoverSolutions(population, problemMatrix, num_elements, num_sets);

    // Libère la mémoire allouée pour les chromosomes des populations
    freeChromosomes(problemMatrix, num_sets);
    free(problemMatrix); // Libération du tableau problemMatrix lui-même

    freeChromosomes(population, POPULATION_SIZE);
    freeChromosomes(children, POPULATION_SIZE);
    freeChromosomes(mutation, mutationCount);
    return 0;
}