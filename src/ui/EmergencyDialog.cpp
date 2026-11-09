#include "ui/EmergencyDialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

EmergencyDialog::EmergencyDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Emergency meeting"));
    setModal(true);

    m_infoLabel = new QLabel(
        tr("Do you want to call an emergency meeting?"), this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setWordWrap(true);

    m_callButton = new QPushButton(tr("CALL EMERGENCY MEETING"), this);
    m_callButton->setStyleSheet(
        "QPushButton { background-color: #d62828; color: white;"
        "font-weight: bold; padding: 12px; }"
        "QPushButton:hover { background-color: #b51f1f; }");

    m_cancelButton = new QPushButton(tr("Cancel"), this);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(m_callButton);
    buttons->addWidget(m_cancelButton);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_infoLabel);
    layout->addLayout(buttons);

    connect(m_callButton, &QPushButton::clicked,
            this, [this]() {
                emit meetingRequested();
                accept();
            });

    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
}
