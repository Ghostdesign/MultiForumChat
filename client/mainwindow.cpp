#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clients.h"
#include<QtCore>
#include<QtGui>
#include<QThread>
#include<QString>
#include<string>
#include<QDebug>
#include<iostream>
#include<QMessageBox>

QThread *myThread;
clients *unClient;

//Constructeur de l'interface principale
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("OUED-MESSENGER");
}

//Destructeur de l'interface.
MainWindow::~MainWindow()
{
    //Si le client est connecté, arrete le thread et deconnecte le client avant d'arreter le programme
    if (unClient != NULL)
    {
        if (myThread->isRunning())
        {
             myThread->terminate();
        }
        unClient->~clients();
        unClient = NULL;
    }
     delete ui;
}

//Connect le client au serveur lorsque le bouton connect est cliqué ou le déconnecte si il est déja connecté

void MainWindow::on_ConnectButton_clicked()
{
   if (ui->ConnectButton->text() == "Se Connecter")
    {
        unClient = new clients();
        myThread = new QThread();

        unClient->connection();
        if (unClient->connected())
        {
            ui->textBrowser->setText("Connexion au serveur reussie! \n");
            ui->ConnectButton->setText("Deconnecter");


            unClient->Dosetup(*myThread);
            unClient->moveToThread(myThread);
            myThread->start(QThread::HighestPriority);

            //On connect le signal newMessageReiceved a la fonction onMsgReceived.
            QObject::connect(unClient,SIGNAL(newMessageReiceved()),this,SLOT(onMsgReceived()));
        }
        else
        {
            QMessageBox::information(this,"Erreur de connection","Serveur non disponible");
        }



   }
   else
   {

       if (unClient != NULL)
       {
           unClient->~clients();
           unClient = NULL;
       }
       if (myThread->isRunning())
       {
            myThread->terminate();
            myThread = NULL;
       }

       ui->ConnectButton->setText("Se Connecter");
       ui->textBrowser->setText("Deconnexion du serveur reussie! \n");

   }

}

// Permet de choisir le forum sport lorsque le bouton Sport est cliqué
// Utilisateur doit etre connecté et ne peut choisir qu'un seul forum
void MainWindow::on_forumSportButton_clicked()
{
    if(unClient != NULL && unClient->connected())
    {
        if (unClient->forum_choisi())
        {
            QMessageBox::information(this,"Forum","Vous avez deja choisi un forum! \n Veuillez vous deconnecter pour choisir un nouveau forum.");
        }
        else
        {
             unClient->choix("sport");
        }
    }
    else
    {
        QMessageBox::information(this,"Erreur de connection","Veuillez vous connecter avant de choisir un forum");
    }


}

// Permet de choisir le forum Culture lorsque le bouton Culture est cliqué
// Utilisateur doit etre connecté et ne peut choisir qu'un seul forum
void MainWindow::on_forumCultureButton_clicked()
{
    if(unClient != NULL && unClient->connected())
    {
        if (unClient->forum_choisi())
        {
            QMessageBox::information(this,"Forum","Vous avez deja choisi un forum! \n Veuillez vous deconnecter pour choisir un nouveau forum.");
        }
        else
        {
             unClient->choix("culture");
        }
    }
    else
    {
        QMessageBox::information(this,"Erreur de connection","Veuillez vous connecter avant de choisir un forum");
    }
}

// Permet de choisir le forum Amitier lorsque le bouton Amitier est cliqué
// Utilisateur doit etre connecté et ne peut choisir qu'un seul forum
void MainWindow::on_forumAmitierButton_clicked()
{
    if(unClient != NULL && unClient->connected())
    {
        if (unClient->forum_choisi())
        {
            QMessageBox::information(this,"Forum","Vous avez deja choisi un forum! \n Veuillez vous deconnecter pour choisir un nouveau forum.");
        }
        else
        {
             unClient->choix("amitier");
        }
    }
    else
    {
        QMessageBox::information(this,"Erreur de connection","Veuillez vous connecter avant de choisir un forum");
    }
}

//Appelé lorsque le bouton send est cliqué. Permet d'envoyer le message saisi par l'utilisateur
void MainWindow::on_SendButton_clicked()
{
  if(unClient != NULL && unClient->connected())
    {
        QString msg = ui->msgTextBox->toPlainText(); //Recupere le message
        QString nickname = ui->nickNameTextBox->toPlainText(); //Recupere le nickname

        if (msg.isEmpty())
        {
           QMessageBox::information(this,"Message Vide","Impossible d'envoyer un message vide");

        }
        else if (nickname.isEmpty())
        {
                QMessageBox::information(this,"Nickname vide","Veuillez choisir un nickname");
        }

        else if (!unClient->forum_choisi())
        {
            QMessageBox::information(this,"Erreur d'envoyer","Veuillez choisir un forum avant de pouvoir envoyer un message");
        }
        else if (msg == "bye" || msg == "Bye")
        {
            if (myThread->isRunning())
            {
                 myThread->terminate();
            }
            if (unClient != NULL)
            {
                unClient->~clients();
                unClient = NULL;
            }
            ui->ConnectButton->setText("Se Connecter");
            ui->textBrowser->setText("Deconnexion du serveur reussie! \n");
        }
        else
        {
            //Associe le nickname et le msg puis envoie l'ensemble dans un msg
             QString msgTosend = nickname + " : " + msg ;

             const char* message = msgTosend.toStdString().c_str(); //Convertie de Qstring à Char[]
             unClient->envoi(message);
             ui->msgTextBox->clear();
        }
    }
  else
  {
      QMessageBox::information(this,"Erreur de connection","Veuillez vous connecter avant de pouvoir envoyer un message");
  }
}

//Permet d'afficher un message dans la boite de texte lorqu'un message est recu.
void MainWindow::onMsgReceived()
{
    const char* sms = unClient->getInbufContent(); //Recupere le message
    QString myString = QString::fromUtf8(sms); //Convertion de char[] en QString

    if (myString == "Serveur Down")  //Si le message recu est serveur Down, deconnecte l'utilisateur
    {

        if (unClient != NULL)
        {
            if (myThread->isRunning())
            {
                 myThread->terminate();
            }

            unClient->closeSocket();
            unClient->~clients();
            unClient = NULL;
        }
        QMessageBox::information(this,"Serveur Down","Le serveur s'est deconnecte!");
        ui->ConnectButton->setText("Se Connecter");
        ui->textBrowser->setText("Serveur deconnecter! \n");

    }
    else //Sinon affiche le message
    {
        ui->textBrowser->append(myString);
    }
}
