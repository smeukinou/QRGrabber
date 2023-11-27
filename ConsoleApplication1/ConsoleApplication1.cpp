// ConsoleApplication1.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <windows.h>

typedef int (*goCallback)(const char*, int);

int goCallback2(const char* c, int) {
    std::cout << " result: " << c << std::endl;
    return 0;
}

int main()
{
    HINSTANCE hLib;
    FARPROC function;

    // Load the DLL
    hLib = LoadLibrary(L"QRGrabber.x86.dll"); // Replace "example.dll" with your DLL name
    if (hLib == NULL) {
        printf("Error loading the library\n");
        return 1;
    }

    // Get the function address
    function = GetProcAddress(hLib, "monitor"); // Replace "FunctionName" with the function you want to get
    if (function == NULL) {
        printf("Error getting function address\n");
        return 1;
    }

    
     int (*func)(const char*, int,goCallback) = (int (*)(const char *,int, goCallback))function;

     func("1", 1, goCallback2);

     func(NULL, 0, goCallback2);

     for(int i=0;i<3;i++){
         Sleep(5 * 1000);
         func("2", 1, goCallback2);
     }

     func("0", 1, goCallback2);

    // Unload the DLL when done
    if (!FreeLibrary(hLib)) {
        printf("Error unloading the library\n");
        return 1;
    }
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
