#ifndef STARTSCREEN_H
#define STARTSCREEN_H

// ============================================================================
//  StartScreen
//  Pocetni ekran: unos imena i IP adrese, dugmad Host i Join.
//
//  Dijagram:
//    - m_nameEdit, m_ipEdit: QLineEdit*
//    - m_hostBtn, m_joinBtn: QPushButton*
//    <<signal>> hostClicked(name)
//    <<signal>> joinClicked(name, ip)
// ============================================================================

#include <QWidget>

class QLineEdit;
class QPushButton;

class StartScreen : public QWidget
{
    Q_OBJECT

public:
    explicit StartScreen(QWidget* parent = nullptr);

    // Ponovo omogucava dugmad (npr. posle neuspelog povezivanja).
    void setBusy(bool busy);

signals:
    void hostClicked(const QString& name);
    void joinClicked(const QString& name, const QString& ip);

private slots:
    void updateButtons();

private:
    QLineEdit*   m_nameEdit;
    QLineEdit*   m_ipEdit;
    QPushButton* m_hostBtn;
    QPushButton* m_joinBtn;
    bool         m_busy = false;
};

#endif // STARTSCREEN_H
