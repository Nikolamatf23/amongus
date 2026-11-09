#ifndef EMERGENCYDIALOG_H
#define EMERGENCYDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class EmergencyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmergencyDialog(QWidget* parent = nullptr);

signals:
    void meetingRequested();

private:
    QLabel* m_infoLabel = nullptr;
    QPushButton* m_callButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
};

#endif // EMERGENCYDIALOG_H
