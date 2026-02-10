/* @file Окошко для отображения истории инкассаций. */

#include "EncashmentHistoryWindow.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>

#include "Backend/PaymentManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "ServiceTags.h"

EncashmentHistoryWindow::EncashmentHistoryWindow(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QWidget(aParent), m_Backend(aBackend) {
    setupUi(this);

    m_SignalMapper = new QSignalMapper(this);
    connect(m_SignalMapper, SIGNAL(mapped(int)), this, SLOT(printEncashment(int)));
}

//------------------------------------------------------------------------
EncashmentHistoryWindow::~EncashmentHistoryWindow() {}

//------------------------------------------------------------------------
void EncashmentHistoryWindow::updateHistory() {
    // clear all
    foreach (auto button, m_Widgets) {
        gridHistoryLayout->removeWidget(button);
        button->deleteLater();
    }
    m_Widgets.clear();

    auto paymentManager = m_Backend->getPaymentManager();
    int count = paymentManager->getEncashmentsHistoryCount();

    for (int i = 0; i < count; i++) {
        QVariantMap encashment = paymentManager->getEncashmentInfo(i);

        QString text = QString("[%1] %2\n")
                           .arg(encashment[CServiceTags::EncashmentID].toString())
                           .arg(encashment[CServiceTags::EncashmentDate].toString()) +
                       tr("#total") + ": " + encashment[CServiceTags::CashAmount].toString();

        QPushButton *button = new QPushButton(text, this);
        button->setMinimumHeight(35);
        button->setMaximumWidth(250);
        button->setEnabled(
            paymentManager->canPrint(SDK::PaymentProcessor::CReceiptType::Encashment));

        connect(button, SIGNAL(clicked()), m_SignalMapper, SLOT(map()));
        m_SignalMapper->setMapping(button, i);

        gridHistoryLayout->addWidget(button, i % 5, i / 5);
        m_Widgets << button;
    }

    gridHistoryLayout->addItem(
        new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), 5, 0);
}

//------------------------------------------------------------------------
void EncashmentHistoryWindow::printEncashment(int aIndex) {
    auto paymentManager = m_Backend->getPaymentManager();

    paymentManager->printEncashment(aIndex);
}

//------------------------------------------------------------------------
