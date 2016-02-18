#include "clients.h"
//== programme client.cpp =====

// Le processus client doit réaliser les operations suivantes :
//		- Créer une socket
//		- Se connecter au processus serveur en précisant son identité (Adresse internet et le numéro de port). Cette opération attribue automatiquement un numéro de port local au processus client.
//	    - Lire ou écrire sur la voie de communication identifie par la socket.
//	    - Fermer la socket


// Fichier Include
#include <string>
#include <iostream>
#include <netinet/in.h> // librairie qui definie les structures sockaddr_in et in_addr
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<QObject>
#include <errno.h>


using namespace std;

// Defines
#define  PORT_NUM         1050
#define  IP_ADDR "139.103.84.19"  // Definis l'adresse du serveur

//Constructeur de la classe client
//Initialise les parametres de connection
clients::clients(QObject *parent) :
    QObject(parent)
{

  // Creation (ouverutre) de la socket de type datastream
    server_s = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family      = AF_INET;   //  Specifie la famille de protocole à laquelle appartient l'adresse (AF_INET ici pour TCP/IP)
    server_addr.sin_port        = htons(PORT_NUM);   // Specifie le numero de port de l'application
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR); // Specifie l'adrese du serveur
    choice = false;
    connecteD = false;
    SockState = false;
}

//Permet d'initialiser la connexion
void clients::connection()
{

      if ( ::connect(server_s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
      {
          cout << errno <<endl;;
      }
      else
      {
          cout << "connect" << endl ;
          connecteD = true;
          SockState = false;
      }
}

// Copie le message a envoyer dans le buffer et envoi le message a travers la commande send()
// qui envoie les données en mode connecté.
void clients::envoi(const char* msg)
{
   send(server_s, msg, (strlen(msg) + 1), 0);
}

//Permet d'envoyer le choix du forum au serveur
void clients::choix(string c)
{
    choix_forum = c.c_str();
    send(server_s, choix_forum , (strlen(choix_forum ) + 1), 0);
    choice = true;

}

//Fonction utilisée par un thread pour recevoir les messages venant de l'exterieur
void clients::recevoir()
{
    // Recoie et affiche les donnees envoyeés par le serveur dans un buffer in_buf à travers la commande recv()
    while( recv(server_s, in_buf, sizeof(in_buf), 0) > 0)
    {
        emit newMessageReiceved(); // emet un signal lorsqu'un message est recu
    }
}

//Retourne le contenu du buffer d'entré
const char* clients::getInbufContent()
{
    return in_buf;
}

//Initialise le thread qui a pour fonction de gerer la reception de message
void clients::Dosetup(QThread &cThread)
{
    QObject::connect(&cThread,SIGNAL(started()),this,SLOT(recevoir()));
}

//Permet de savoir si un forum a deja été choisi ou non
bool clients::forum_choisi()
{
    return choice;
}
//Permet de savoir si le client est connecté ou non
bool clients::connected()
{
    return connecteD;
}

//Permet de fermer la socket du serveur
void clients::closeSocket()
{
    close(server_s);
    SockState = true;
}

//Destructeur de la classe.
clients::~clients()
{
 // Si la socket n'est pas fermé, on envoi un message de deconnection au serveur et on ferme la socket
    if (SockState == false)
    {
       send(server_s, "Nickname : bye" , 15 , 0);
       close(server_s);
    }
}

