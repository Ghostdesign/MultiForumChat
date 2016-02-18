#ifndef CLIENTS_H
#define CLIENTS_H

#include <netinet/in.h> // librairie qui definie les structures sockaddr_in et in_addr
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <QObject>
#include<QThread>
#include <string>

class clients : public QObject
{
     Q_OBJECT

private:
        //Variables globales de connection
          struct sockaddr_in   server_addr;     // la structure d'adresse specifique au domaine
          unsigned int         server_s;        // identificateur de la socket
          unsigned int         addr_len;        // Recupère la taille de la structure d'adresse du client
          char                 out_buf[10000];    // buffer de sortie
          char                 in_buf[10000];     // buffer d'entrée
          //variables globale de gestions
          const char *choix_forum;
          bool choice;
          bool connecteD;


public:
          //Toutes les descriptions des methodes sont definies dans la definition des methodes "fichier clients.cpp"
    explicit clients(QObject *parent = 0);
    ~clients();
    void connection();
    void envoi(const char*);
    void choix(std::string);
    const char* getInbufContent();
    void Dosetup(QThread &cThread);
    bool forum_choisi();
    bool connected();
    void closeSocket();
    bool SockState;

signals:
    void newMessageReiceved(); //Signal emit par le thread lorsqu'un message est recu.
public slots:
    void recevoir(); //Fonction executée par le thread
};

#endif // CLIENTS_H
