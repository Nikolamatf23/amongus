#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QMetaObject>

#include "ui/GameScreen.h"
#include "network/Client.h"
#include "network/ClientController.h"
#include "game/GameState.h"
#include "game/Player.h"
#include "game/DeadBody.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GameState state;
    Client client;
    ClientController controller(&client, &state);

    state.setPhase(GamePhase::Playing);

    QPixmap mapPixmap(":/map/background.jpg");
    const QSize mapSize = mapPixmap.isNull()
                              ? QSize(1200, 800)
                              : mapPixmap.size();

    const qreal centerX = mapSize.width() / 2.0;
    const qreal centerY = mapSize.height() / 2.0;

    for (int i = 1; i <= 6; ++i) {
        Player player(i, QString("Player %1").arg(i));
        player.setSkinId(i);
        state.addPlayer(player);

        const qreal x = centerX + (i - 3) * 70.0;
        const qreal y = centerY + (i % 2 == 0 ? 80.0 : -80.0);
        state.player(i)->setPos(QPointF(x, y));
    }

    state.player(3)->setAlive(false);
    state.addBody(DeadBody(3, state.player(3)->pos()));

    GameScreen screen(&controller);
    screen.setCameraTargetPlayerId(1); // lokalni igrač u standalone testu
    screen.setWindowTitle("GameScreen - test");
    screen.resize(1100, 750);
    screen.show();

    QTimer animationTimer;
    const qreal startingX = state.player(1)->pos().x();
    bool movingRight = true;

    QObject::connect(&animationTimer, &QTimer::timeout, [&]() {
        Player* player = state.player(1);
        if (player == nullptr) {
            return;
        }

        QPointF position = player->pos();

        if (movingRight) {
            position.rx() += 5.0;
            if (position.x() >= startingX + 160.0) {
                movingRight = false;
            }
        } else {
            position.rx() -= 5.0;
            if (position.x() <= startingX - 160.0) {
                movingRight = true;
            }
        }

        player->setPos(position);
        QMetaObject::invokeMethod(
            &controller, "stateChanged", Qt::DirectConnection);
    });

    animationTimer.start(100);
    return app.exec();
}
