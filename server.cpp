//== programme server.cpp =====

// Le processus Server doit réaliser les operations suivantes :
//		- Créer une file de message
//		- Envoie ou recevoir des messages par la file de message
//		- Accepter les nouvelles connections en creant de nouveau processus
//		- Gerer les depart des clients et l'arret du Serveur
//		- Gerer la distribution des messages au bon forum
//	    - Lire ou écrire sur la voie de communication identifie par la socket.
//	    - Fermer la socket

//Fichier Include
#include <iostream>
#include <errno.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <time.h>

#include <sys/types.h>

#include <vector>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <algorithm>
#include <iterator>
#include <sstream>


using namespace std;

#define MSG_R 400//droit d'acces a la file en lecture
#define MSG_W 400//droit d'acces a la file en ecriture
#define CLE 226// identificateur unique de la clé de la file de message
#define MSG_SIZE_TEXT 10000// definition de la taille maximal d'un message

#define PORT_NUM    1050 // on doit definir le numero de port (toute valeur > 1024 ex: 6000 7000)
#define MAX_LISTEN    10//  Le maximum de personne qui peuvent etre dans la file d'attente de la socket s'il y'a plus de 10 le server sera occupé


int msqid ;//variable qui contient l'identificateur unique la file de message

/*Vecteur d'entier qui stocke les idenifiants
 unique des clients qui se connecte au serveur
*/
vector<int> listConecte;// contient les idenifiants de tous les clients

vector<int> culture;// contient les idenifiants des clients du forum Culture
vector<int> sport;// contient les idenifiants des clients du forum Sport
vector<int> amitie;// contient les idenifiants des clients du forum amitie

/*
Iterateur pour optimiser la recherche
des identifiant client a supprmer lors des deconnection
*/
vector<int>::iterator position;
vector<int>::iterator position2;
vector<int>::iterator position3;


unsigned int         server_s;// c'est un descripteur socket d'ecoute
struct sockaddr_in   server_addr;// c'est une structure c'est fait pour definir les information relatif au serveur une adress, un numero de port, type de protocol a la socket
unsigned int         client_s;// c'est la socket avec laquel le serveur communique avec le client
unsigned int         client_to_close;// c'est la socket avec laquel le serveur ferme la comunication avec des clients en parallele
struct sockaddr_in   client_addr;// c'est une structure c'est fait pour definir les information relatif au client une adress, un numero de port, type de protocol a la socket
unsigned int         addr_len;// longueur de l'adress necessaire pour le passage en parametre de la fonction bind
char                 out_buf[10000];// premier tableau pour metre de l'information et envoie au client
char                 in_buf[10000];// deuxieme tableau pour recption des messages qui seront recu par le serveur


/*
Structure du message qui contient le tableau ou est stocker
le message ainsi que son type
*/
struct msgtext
{
    long mtype ;
    char mtext[MSG_SIZE_TEXT] ;
} msg ; // msg structure associée au message

//cette variable est utiliser pour stocker le message a envoie a tout les membre du forum destinataire
char spreadmsg[MSG_SIZE_TEXT];

//Conversion de char en unsigned int en effet on fait cette conversion pour determiner qu'elle client deconnecter Cela est fait a cause des fork
//On recoit le message de deconnection au niveau du fils et on envoie l'identifiant en char au pere pour fermer la socket correspondante
unsigned int toUnsignedInt (char x[])
{
  stringstream ss;
  ss << x;
  unsigned int y;
  ss >> y;
  return y;
}

//Signaux Pour prevenir le pere d'aller lire dans la file

void my_handler(int signum)
{
	msgtext outmsg;
	outmsg.mtype = 1;
	
    if (signum == SIGUSR1)
    {
       //On lit le message
        if (msgrcv(msqid, &outmsg, sizeof(outmsg.mtext), 1, 0) < 0)
        {  
            perror("msgrcv");//gestion d'erreur en cas d'ereur de lecture
            msgctl(msqid, IPC_RMID, NULL);
            exit(1);
        }
	strcpy(spreadmsg,outmsg.mtext) ;
       // cout << "msg.mtext : " << outmsg.mtext << endl;
        cout<<endl;
        cout<<"je suis le Pere J'ai lu ce msg: ----> "<< spreadmsg <<endl;
        
        
        
        //On lit le forum
         if (msgrcv(msqid, &outmsg, sizeof(outmsg.mtext), 1, 0) < 0)
        {  
            perror("msgrcv");
            msgctl(msqid, IPC_RMID, NULL);
            exit(1);
        }
        char forum[10];
	strcpy(forum,outmsg.mtext) ;

        if (strcmp(forum,"culture") == 0)//On envoie le message a tous les clients du forum Culture
        {
            for (int i=0; i<culture.size(); i++)
            {
                send(culture[i], spreadmsg, (strlen(spreadmsg) + 1), 0);	
            }
        }
        else if (strcmp(forum,"sport") == 0)//On envoie le message a tous les clients du forum Sport
        {  
            for (int i=0; i<sport.size(); i++)
            {
                send(sport[i], spreadmsg, (strlen(spreadmsg) + 1), 0);
            }
        }
        else if (strcmp(forum,"amitie") == 0)//On envoie le message a tous les clients du forum Amitie
        {
            for (int i=0; i<amitie.size(); i++)
            {
                send(amitie[i], spreadmsg, (strlen(spreadmsg) + 1), 0);
            }
        }
        
        
    }
}

//gestion pour CTRl_C

void handlerCtrlC(int numsig)
{
    if (numsig == SIGINT)
    {
        strcpy(out_buf, "Serveur Down");//copy du message en envoyer a tous les clients
        for (int i=0; i<listConecte.size(); i++)
        {
            send(listConecte[i], out_buf, (strlen(out_buf) + 1), 0);//envoie a tous les clients
            close(listConecte[i]);
        }
        if (msgctl(msqid, IPC_RMID, NULL) == -1)//suppression de la file de message avec msgctl
        {
            fprintf(stderr, "Message queue could not be deleted.\n");//message d'erreur en cas dechec
            exit(EXIT_FAILURE);
        }
        close(server_s);//fermeture de la socket du server
        cout<<"Message queue was deleted.\n";
        exit(1);
    }
}
//gestion pour CTRl_C_fils

void handlerCtrlC_fils(int numsig)
{
    if (numsig == SIGINT)
    {
        close(client_s);
        exit(1);
    }
}

//gestion bye
void handlerBye(int numb)
{  
    msgtext clientsocket;
    clientsocket.mtype = 2;
    
    if (numb == SIGUSR2)
        
    {
    	if (msgrcv(msqid, &clientsocket, sizeof(clientsocket.mtext), 2, 0) < 0)//on lit le message dans la file de message
        {  
            perror("msgrcv");
            msgctl(msqid, IPC_RMID, NULL);
            exit(1);
        }
	strcpy(spreadmsg,clientsocket.mtext) ;//On envoie le message a tous les autre client avant le deconnecter
	
	client_to_close = toUnsignedInt(spreadmsg);//conversion de l'id du client a deconecter

        cout<<"Reception de demande de depart\n";
        close(client_to_close);//On ferme la socket du client
        listConecte.erase(remove(listConecte.begin(), listConecte.end(), client_to_close), listConecte.end());//on le retire de notre liste de tous les connecter

        cout << endl;
        position =find(culture.begin(), culture.end(), client_to_close);//On le recherche dans le forum culture
        position2 =find(sport.begin(), sport.end(), client_to_close);//On le recherche dans le forum Sport
        position3 =find(amitie.begin(), amitie.end(), client_to_close);//On le recherche dans le forum amitie
        // Pour le forum dans le quel il se trouve supprimer l'element du tableau ou on a trouver l'identifiant du client
        if (position != culture.end())
            culture.erase(position);
        
        if (position2 != sport.end())
            sport.erase(position2);
        
        if (position3 != amitie.end())
            amitie.erase(position3);

    }
}

int main()
{
    
    /******************************
    Gestion des signaux
    *******************************/
    signal(SIGUSR1, my_handler);
    signal(SIGUSR2, handlerBye);
    signal(SIGINT, handlerCtrlC);
    
    
    //Creation de la file de message
    if (( msqid = msgget (CLE , IPC_CREAT|IPC_EXCL| MSG_R |MSG_W)) == -1)
  	 {
         perror("Echec de msgget") ;
         exit(1) ;
     }
    cout <<"identificateur de la file:"<< msqid<<endl ;
    cout << "cette file est identifiée par la clé unique :"<< CLE<<endl ;
    cout<<endl;
    
    server_s = socket(AF_INET, SOCK_STREAM, 0);//parametre domaine , le type de mode TCP ( SOCK_STREAM), toujours zero pour savoir que ces TCP
    
    server_addr.sin_family      = AF_INET;// le domaine internet(AF_INET) local ces autre chose
    server_addr.sin_port        = htons(PORT_NUM);// assigner le numero le port. htons est fait pour convertir le numero de port pour etre comprehensible pour le resau
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);// PErmet de choisir l'adress IP si on doit faire le choix entre plusieurs.
    bind(server_s, (struct sockaddr *)&server_addr, sizeof(server_addr));//on associe a la socket tout les inormation initialisés
    listen(server_s, MAX_LISTEN);//C'est une fonction qui ecoute le resau en attente de demande de connection en fonction du nombre Maximun definit
    
    
    while(1)
    {
        addr_len = sizeof(client_addr);//on recupere la taille de l'adresse du client
        client_s = accept(server_s, (struct sockaddr *)&client_addr, &addr_len);//La fonction accept extrait le premier de la file d'attente crée un nouveau socket connecté et retourne un nouveau descripteur de fichier se référant à cette socket.
        cout<<"Connecter " << client_s ;
        recv(client_s, in_buf, sizeof(in_buf), 0);//On utilise la fonction recv pour recevoir des message a travers les sockets
        
        char choix[100];//le choix d'un forum au niveau du client engendre l'envoie d'un caractere pour determiner le forum ou il veut se connecter
        strcpy(choix,in_buf);
        
        if (strcmp(choix,"culture")==0)
        {
            culture.push_back(client_s);
        }
        else if (strcmp(choix,"sport")==0)
        {
            sport.push_back(client_s);
        }
        else if (strcmp(choix,"amitie")==0)
        {
            amitie.push_back(client_s);
        }

        listConecte.push_back(client_s);//Peut importe le forum on stocke tous les id des client connectee sur le serveur dans ce vecteur
        
        if(fork()==0)//creation de fils a chaque nouveau client
        {
            close(server_s);
            signal(SIGINT, handlerCtrlC_fils);
            //On utilise la fonction recv pour recevoir des message a travers les sockets
            while (recv(client_s, in_buf, sizeof(in_buf), 0) > 0)
            {
   
                //ON recupere la taille du nickmane
                string inbuf_temp = in_buf;
                size_t pos = inbuf_temp.find_first_of(":");         
                inbuf_temp = inbuf_temp.substr(pos+2);
                
                
                if (inbuf_temp != "bye" && inbuf_temp != "Bye" )
                {
                   
                    msg.mtype = 1 ; // type des messages
                    strcpy(msg.mtext, in_buf);


                    // on envoie le message a la file
		  if(msgsnd(msqid,&msg,strlen(msg.mtext)+1,  IPC_NOWAIT) == -1)
                    {
                        perror("impossible d'envoyer le message") ;
                        exit(-1) ;
                    }
                    
                    strcpy(msg.mtext,"");
                    
                   //On envoie le nom du forum a la file
                 strcpy(msg.mtext,choix);
                 if(msgsnd(msqid,&msg,strlen(msg.mtext)+1,  IPC_NOWAIT) == -1)
                    {
                        perror("impossible d'envoyer le message") ;
                        exit(-1) ;
                    }   
                    strcpy(msg.mtext,"");
                    
                    //Previent le pere
                    kill(getppid(), SIGUSR1);
                }
                
                else// si le client veut se deconnecter avec un bye
                {
                 //Envoi le numero de socket du client qui veut se deconnecter au père 
                 msg.mtype = 2 ;
                 stringstream sockclient;
                 char sock[10];
                 sockclient << client_s;
                 sockclient >> sock;
                 strcpy(msg.mtext, sock);
    		 if(msgsnd(msqid,&msg,strlen(msg.mtext)+1,  IPC_NOWAIT) == -1)
                    {
                        perror("impossible d'envoyer le message") ;
                        exit(-1) ;
                    }
                
                 strcpy(msg.mtext,"");
                 
                    kill(getppid(), SIGUSR2);//on previens le pere qu'un processus veut se deconnecter
                    close(client_s);
                    exit(0);       
                }
                
            }
        }


    }

    close(server_s);//fermeture socket server
    return(0);
}
