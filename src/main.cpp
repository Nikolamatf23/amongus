#include "mainwindow.h"

#include "game/ChatMessage.h"
#include "game/GameTypes.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Tipovi koji idu kao parametri signala (ClientController -> UI).
    qRegisterMetaType<GamePhase>();
    qRegisterMetaType<Winner>();
    qRegisterMetaType<ChatMessage>();

    MainWindow w;
    w.show();
    return QApplication::exec();
}
