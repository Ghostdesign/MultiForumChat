#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<clients.h>
#include<QThread>

namespace Ui {
class MainWindow;
}
// Header de l'interface
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void Dosetup(QThread &cThread); //Initialise le thread

private slots:
     void on_ConnectButton_clicked(); // Fonction appelée lorsqu'on clique sur le bouton Connecter

     void on_forumSportButton_clicked(); // Fonction appelée lorsqu'on clique sur le bouton Sport

     void on_forumCultureButton_clicked(); // Fonction appelée lorsqu'on clique sur le bouton Culture

     void on_forumAmitierButton_clicked(); // Fonction appelée lorsqu'on clique sur le bouton Amitier

     void on_SendButton_clicked(); // Fonction appelée lorsqu'on clique sur le bouton Envoyer

private:
    Ui::MainWindow *ui;

public slots:
   void onMsgReceived(); // Affiche le contenu du buffer lorsqu'un msg est recu
};

#endif // MAINWINDOW_H
