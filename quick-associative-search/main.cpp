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

#define OPTIMAL
#define EXPERIMENTAL

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

struct EntreeAnnuaireVec;
bool compare(const EntreeAnnuaireVec& lhs, const EntreeAnnuaireVec& rhs);

struct EntreeAnnuaireVec
{
	std::string nom;
	uint32 numero;
    uint32 hash;

	//
	// operateurs de comparaison appeles implicitement par les fonctions 
	// std::sort et std::lower_bound. Ceci est necessaire afin de determiner 
	// le critere sur lequel on doit trier cette structure.
	// 'lhs' est l'acronyme de "left-hand side" cad la partie gauche de l'expression
	// et 'rhs' celui de "right-hand side" cad la partie droite de l'expression
	//
	inline bool operator<(const EntreeAnnuaireVec& rhs)
	{
		return (hash < rhs.hash);
	}

	inline bool operator<(const std::string& rhs)
	{
		return (nom < rhs);
	}

    inline bool operator<(const uint32& rhs)
	{
		return (hash < rhs);
	}

	EntreeAnnuaireVec() : nom(std::string()), numero(42)
	{
	}
	
	EntreeAnnuaireVec(const EntreeAnnuaireVec &other) : nom(other.nom), numero(other.numero), hash(other.hash)
	{
	}

	// constructeur avec Operateur Move (C++11 seulement)
	// ceci permet d'éviter de copier (et donc d'allouer) les objets en copiant/swappant
	// le contenu de l'objet EntreeAnnuaireVec passe en parametre avec this
	EntreeAnnuaireVec(const EntreeAnnuaireVec &&other) : nom(std::move(other.nom))
													, numero(std::move(other.numero)), hash(std::move(other.hash))
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
        hash = std::move(rhs.hash);
		return (*this);
	}

	// Cette fonction est reconnue automatiquement par les conteneurs et algorithmes de la STL
	// Comme la STL utilise les templates C++ (le 'T' dans Standard Template Library) et que 
	// les templates sont generes a la compilation les algorithmes de tri laissent au compilateur
	// le fait de determiner si T::swap() existe. 
	// Si c'est le cas c'est le cas c'est cette fonction qui sera utilisee sinon celle par defaut.
	/*friend void swap(EntreeAnnuaireVec &left, EntreeAnnuaireVec &right)
	{
		//std::swap(left.nom, right.nom);
		//std::swap(left.numero, right.numero);
		left.nom = std::move(right.nom);
		left.numero = std::move(right.numero);
        left.hash = std::move(right.hash);
	}*/

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

	
	// A ECRIRE  -->
	//
	// insertion sort tenant compte de l'utilisation de cette classe lors de l'initialisation
	//
	static void sort(std::vector<EntreeAnnuaireVec>::iterator begin, std::vector<EntreeAnnuaireVec>::iterator end)
	{
        //on set la valeur de end sur end-1 pour etre dans les bounds du vecteur
        if(begin == --end) return;
        
        //on itere le vecteur à partir de début+1 jusqu'à end
        for (std::vector<EntreeAnnuaireVec>::iterator i = begin + 1; i < end; ++i)
        {
            //pour chaque valeur, on itere depuis cette valeur jusqu'au début du vecteur et on compare les hash
            for(std::vector<EntreeAnnuaireVec>::iterator j = i; j > begin && j->hash < (j - 1)->hash; --j )
            {
                //si la valeur précédente est inférieure, on swap avec la valeur courante
                std::swap(*(j - 1), *j);
            }
        }
        //autres essais
        /*
        for (std::vector<EntreeAnnuaireVec>::iterator it = begin; it != end; ++it)
        {
            std::vector<EntreeAnnuaireVec>::iterator ins = std::upper_bound(begin, it, *it, compare);
            std::rotate(ins, it, std::next(it));
        }*/
        /*
        std::vector<EntreeAnnuaireVec>::iterator min = begin;  
        for(std::vector<EntreeAnnuaireVec>::iterator i = begin + 1; i < end; ++i )  
            if ( *i < *min )  
                min = i;  
  
        std::iter_swap( begin, min );  
        while( ++begin < end )  
            for( std::vector<EntreeAnnuaireVec>::iterator j = begin; *j < *(j - 1); --j )  
                std::iter_swap( (j - 1), j ); 
        */
        //Tri par insertion basique
        /*
        for (std::vector<EntreeAnnuaireVec>::iterator i = begin + 1; i < end; ++i)
        {
            for(std::vector<EntreeAnnuaireVec>::iterator j = i; j->nom < (j - 1)->nom && j > begin; --j )
            {
                std::iter_swap((j - 1), j);
            }
        }*/
	}
	// <-- A ECRIRE

    /*
    * Sort sur l'insert plutôt qu'un sort post insert, pour effectuer un tri uniquement pour l'élément qu'on souhaite insérer
    * cela enlève de la complexité
    */
    static void push(std::vector<EntreeAnnuaireVec> &annuaire, const EntreeAnnuaireVec &entree){
        /* DICHOTOMIE
        */
        bool found = false;
        int start = 0;
        int end = annuaire.size();
        int middle = 0;
        int lookfor = entree.hash;
        while(!found && ((end - start) > 1)){
            middle = (start + end)/2;
            found = (annuaire[middle].hash == lookfor);
            if(annuaire[middle].hash > lookfor) end = middle;
            else start = middle;
        }

        if(found){
            annuaire.insert(annuaire.begin()+middle, entree);
        }else{
            if(annuaire.size() > 0 && annuaire[0].hash > entree.hash)
                annuaire.insert(annuaire.begin(), entree);
            else
                annuaire.push_back(entree);
        }

        /* AVEC ITERATORS
        auto it = annuaire.end()-1;
        for(it; it != annuaire.begin() && it->hash < (it-1)->hash; --it);
        if(it != annuaire.begin()) annuaire.insert(it, entree);
        */
        /* A LA MAIN
        unsigned int i = 0;
        while(annuaire.size() > i && annuaire[i] < (EntreeAnnuaireVec&)entree){
            i++;
        }
        annuaire.insert(annuaire.begin()+i, entree);
        */
    }
};

bool compare(const EntreeAnnuaireVec& lhs, const EntreeAnnuaireVec& rhs){
    return (lhs.nom < rhs.nom);
}

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
        uint32 hash = GetHash(nom);

		// 2. ajout de l'entree dans les annuaires
		EntreeAnnuaire entree;	
		entree.first = nom;
		entree.second = entier;
		annuaire.insert(entree);

		EntreeAnnuaireVec entree_vec;
		entree_vec.nom = nom;
		entree_vec.numero = entier;
        entree_vec.hash = hash;

		// A OPTIMISER -->
		//
		// par un insertion sort tenant compte de l'ordre d'insertion
#ifndef OPTIMAL
        annuaire_vec.push_back(entree_vec);
		std::sort(annuaire_vec.begin(), annuaire_vec.end());
#else
#ifdef EXPERIMENTAL
        EntreeAnnuaireVec::push(annuaire_vec, entree_vec);
#else
        annuaire_vec.push_back(entree_vec);
        EntreeAnnuaireVec::sort(annuaire_vec.begin(), annuaire_vec.end());
        //EntreeAnnuaireVec::push(annuaire_vec, entree_vec);
#endif
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
        //std::vector<EntreeAnnuaireVec>::iterator iter = std::lower_bound(annuaire_vec.begin(), annuaire_vec.end(), GetHash(liste_de_noms[random].c_str()));
		// <--- A OPTIMISER

        /*
        * Recherche dichotomique
        */
        bool found = false;
        int start = 0;
        int end = annuaire_vec.size();
        int middle = 0;
        int lookfor = GetHash(liste_de_noms[random].c_str());
        while(!found && ((end - start) > 1)){
            middle = (start + end)/2;
            found = (annuaire_vec[middle].hash == lookfor);
            if(annuaire_vec[middle].hash > lookfor) end = middle;
            else start = middle;
        }
		//if (iter != annuaire_vec.end())
		    compteur++;
	}
	benchmark.End();
	printf("compteur = %d, duree de la recherche dans std::map avec hash : %Lf millisecondes\n", compteur, benchmark.GetElapsedTime()*1000.0);

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

