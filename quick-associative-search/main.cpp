// ***************************************************************************
//
// TP 1 - optimisation d'une recherche dans un tableau trie
//
// Nous avons deja vu comment optimiser la recherche dans un tableau (vector)
// en le triant prealablement -a chaque push_back par exemple- puis en appliquant
// une recherche dichotomique (binary search).
//
// Cependant le temps d'initialisation a explose. De plus, et quand bien meme 
// la recherche est plus optimale que la version intiale on se retrouve tout
// de meme a comparer des chaines de caractere a chaque passage.
//
//
// Le TP consiste a :
// -----------------
//
// I. optimiser la partie d'initialisation en remplacant le std::sort par un
//	tri par insertion (plus interessant pour notre cas). Il est important de modifier
//	le code du tri afin de tenir compte de la facon dont les elements sont 
//	inserees au fur et a mesure dans le tableau.
//
// II. optimiser la partie de recherche dans annuaire_vec et plus specifiquement
//	les comparaisons de std::string que l'on remplacera par des comparaisons
//	d'entiers. Pour cela il faut appliquer la fonction de hachage FNV1a afin de 
//	generer une cle de hachage (que l'on suppose unique) sur 32 bits.
//
//  >>> Merci de bien COMMENTER et EXPLIQUER vos choix pour chacun des cas <<<
//
// ***************************************************************************

// --- includes --------------------------------------------------------------

#include <cstdlib>
#include <cstdio>

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <common/EsgiTimer.h>
// --- declarations ----------------------------------------------------------

typedef unsigned char uint8;
typedef unsigned int uint32;

// l'entree dans nos annuaires
typedef std::pair<std::string, uint32> EntreeAnnuaire;

struct EntreeAnnuaireVec
{
	std::string nom;
	uint32 numero;

	//
	// operateurs de comparaison appeles implicitement par les fonctions 
	// std::sort et std::lower_bound. Ceci est necessaire afin de determiner 
	// le critere sur lequel on doit trier cette structure.
	// 'lhs' est l'acronyme de "left-hand side" cad la partie gauche de l'expression
	// et 'rhs' celui de "right-hand side" cad la partie droite de l'expression
	//
	inline bool operator<(const EntreeAnnuaireVec& rhs)
	{
		return (nom < rhs.nom);
	}

	inline bool operator<(const std::string& rhs)
	{
		return (nom < rhs);
	}

	EntreeAnnuaireVec() : nom(std::string()), numero(42)
	{
	}
	
	EntreeAnnuaireVec(const EntreeAnnuaireVec &other) : nom(other.nom), numero(other.numero)
	{
	}

	// constructeur avec Operateur Move (C++11 seulement)
	// ceci permet d'éviter de copier (et donc d'allouer) les objets en copiant/swappant
	// le contenu de l'objet EntreeAnnuaireVec passe en parametre avec this
	EntreeAnnuaireVec(const EntreeAnnuaireVec &&other) : nom(std::move(other.nom))
													, numero(std::move(other.numero))
	{
	}

	// Effectue la meme que le constructeur precedent mais avec l'operateur d'affectation
	// ces deux fonctions utilisant des references rvalue permettent d'éviter totalement 
	// d'avoir recours à des allocations memoires pendant la phase de tri
	// (puisqu'un tri effectue au minimum la creation d'une variable temporaire et un swap)
	EntreeAnnuaireVec& operator=(EntreeAnnuaireVec &&rhs)
	{
		nom = std::move(rhs.nom);
		numero = std::move(rhs.numero);		
		return (*this);
	}

	// Cette fonction est reconnue automatiquement par les conteneurs et algorithmes de la STL
	// Comme la STL utilise les templates C++ (le 'T' dans Standard Template Library) et que 
	// les templates sont generes a la compilation les algorithmes de tri laissent au compilateur
	// le fait de determiner si T::swap() existe. 
	// Si c'est le cas c'est le cas c'est cette fonction qui sera utilisee sinon celle par defaut.
	friend void swap(EntreeAnnuaireVec &left, EntreeAnnuaireVec &right)
	{
		//std::swap(left.nom, right.nom);
		//std::swap(left.numero, right.numero);
		left.nom = std::move(right.nom);
		left.numero = std::move(right.numero);
	}

	// Ceci nous avait servi lors de la premiere tentative d'optimisation en C++03
	// en utilisant l'idiome "Copy and Swap" qui consiste a passer le parametre par valeur
	// et a lui voler (vampiriser) ses donnees pour les affecter a this (a l'aide de swap).
	// 
	// En C++ les references rvalue (&&) ainsi que l'operateur move (std::move) permettent de faire
	// Ceci de maniere plus optimal.
	//
	//EntreeAnnuaireVec& operator=(EntreeAnnuaireVec other)
	//{
	//	swap(*this, other);
	//	return (*this);
	//}

	/*
	// A ECRIRE  -->
	//
	// insertion sort tenant compte de l'utilisation de cette classe lors de l'initialisation
	//
	static void sort(std::vector<EntreeAnnuaireVec>::iterator begin, std::vector<EntreeAnnuaireVec>::iterator end)
	{
	}
	// <-- A ECRIRE*/
};

/*
template<> inline void std::swap<EntreeAnnuaireVec>(EntreeAnnuaireVec& lhs, EntreeAnnuaireVec& rhs)
{
	lhs.swap(rhs);
}
*/


// --- notre fonction de hashage, algorithme FNV1a
// cf. http://www.isthe.com/chongo/tech/comp/fnv/
// plus particulierement http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a

const uint32 fnv_prime_32 = 16777619;
const uint32 fnv_offset_32 = 2166136261;

static uint32 fnv1a(char _byte, uint32 hash)
{
    return (_byte ^ hash) * fnv_prime_32;
}

static uint32 GetHash(const char *string)
{
    uint32 hash = fnv_offset_32;
	int i = 0;
    while (string[i])
    {
        hash = fnv1a(string[i], hash);
		++i;
    }

    return hash;
}

// --- main ------------------------------------------------------------------

#define TAB_SIZE		10000
#define NUMERO_SIZE		8
#define NOM_SIZE		64

int main(int argc, char* argv[])
{	
	// les structures de donnees a comparer
	std::map<std::string, uint32> annuaire;
	std::vector<EntreeAnnuaireVec> annuaire_vec;
	char numero[NUMERO_SIZE];	
	char nom[NOM_SIZE];

	// ceci va nous servir a savoir quoi chercher par la suite
	std::vector<std::string> liste_de_noms;	

	// necessaire afin d'avoir toujours les memes donnees generees
	srand(12777519);
	
	// histoire d'eviter une eventuelle catastrophe...
	numero[NUMERO_SIZE-1] = 0;
	nom[NOM_SIZE-1] = 0;		

	EsgiTimer benchmark;
	benchmark.Begin();

	//
	// initialisation des annuaires
	// 

	for (uint32 index = 0; index < TAB_SIZE; ++index)
	{
		// 1. generation aleatoire 
		for (uint32 chiffre = 0; chiffre < (NUMERO_SIZE-1); ++chiffre) {
			numero[chiffre] = 48+rand()%10;
		}

		uint32 longueur = rand()%(NOM_SIZE-1);
		for (uint32 lettre = 0; lettre < longueur; ++lettre) {
			nom[lettre] = 65+rand()%57;
		}
		nom[longueur] = 0;
		
		uint32 entier = atoi(numero);

		// 2. ajout de l'entree dans les annuaires
		EntreeAnnuaire entree;	
		entree.first = nom;
		entree.second = entier;
		annuaire.insert(entree);

		EntreeAnnuaireVec entree_vec;
		entree_vec.nom = nom;
		entree_vec.numero = entier;
		annuaire_vec.push_back(entree_vec);				
			
		// A OPTIMISER -->
		//
		// par un insertion sort tenant compte de l'ordre d'insertion
#ifndef OPTIMAL
		std::sort(annuaire_vec.begin(), annuaire_vec.end());
#else
		EntreeAnnuaireVec::sort(annuaire_vec.begin(), annuaire_vec.end());
#endif
		// <-- A OPTIMISER

		// 3. on sauvegarde le nom pour la suite
		liste_de_noms.push_back(entree.first);
	}

	benchmark.End();
	printf("duree de l'initialisation : %Lf millisecondes\n", benchmark.GetElapsedTime()*1000.0);

	//
	// recherche aleatoire d'element dans le tableau
	//

	srand(127);
	uint32 compteur = 0;
	benchmark.Begin();
	for (uint32 index = 0; index < TAB_SIZE; ++index)
	{
		uint32 random = (rand()*10)%TAB_SIZE;

		// A OPTIMISER -->
		// ici l'algorithme de recherche dichotomique (binary search) se nomme std::lower_bound.
		// le troisieme parametre de la fonction se trouve etre l'element a rechercher.
		// Or, liste_de_noms[] est un tableau de std::string.
		// Si vous avez optimisez l'initialisation a l'aide des valeurs de hachage il vous faut, 
		// afin d'etre pertinent, optimiser aussi la recherche de ce meme element en evitant
		// comme precedemment d'effectuer des comparaisons de chaines de caracteres.

		std::vector<EntreeAnnuaireVec>::iterator iter = std::lower_bound(annuaire_vec.begin(), annuaire_vec.end(), liste_de_noms[random]);

		// <--- A OPTIMISER

		if (iter != annuaire_vec.end())
			compteur++;
	}
	benchmark.End();
	printf("compteur = %d, duree de la recherche dans std::vector : %Lf millisecondes\n", compteur, benchmark.GetElapsedTime()*1000.0);

	//
	// recherche aleatoire d'element dans la map (implementation de reference pour notre tentative d'optimisation)
	//

	srand(127);
	compteur = 0;
	benchmark.Begin();
	for (uint32 index = 0; index < TAB_SIZE; ++index)
	{
		uint32 random = (rand()*10)%TAB_SIZE;
		std::map<std::string, uint32>::iterator iter = annuaire.find(liste_de_noms[random]);
		if (iter != annuaire.end())
			compteur++;
	}
	benchmark.End();
	printf("compteur = %d, duree de la recherche dans std::map : %Lf millisecondes\n", compteur, benchmark.GetElapsedTime()*1000.0);

	getchar();

	return 0;
}

