/* @file Окно платежей. */

// boost

#include "PaymentServiceWindow.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <QtCore/QItem_SelectionModel>
#include <QtCore/QSettings>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLayout>

#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Step.h>

#include <boost/bind/bind.hpp>

#include "Backend/HumoServiceBackend.h"
#include "Backend/PaymentManager.h"
#include "MessageBox/MessageBox.h"
#include "ServiceTags.h"

namespace PPSDK = SDK::PaymentProcessor;
namespace CPayment = SDK::PaymentProcessor::CPayment::Parameters;

//----------------------------------------------------------------------------
namespace CPaymentServiceWindow {
const QString ColumnVisibility = "columnVisibility";
}; // namespace CPaymentServiceWindow

//----------------------------------------------------------------------------
PaymentServiceWindow::PaymentServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend), m_Backend(aBackend), m_FiscalMode(false) {
    setupUi(this);

    m_PaymentManager = m_Backend->getPaymentManager();
    m_Model = new PaymentTableModel(m_FiscalMode, m_PaymentManager, this);
    m_ProxyModel = new PaymentProxyModel(this);
    m_ProxyModel->setDynamicSortFilter(true);
    m_ProxyModel->setSourceModel(m_Model);
    tvPayments->setModel(m_ProxyModel);

    tvPayments->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tvPayments->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(m_ProxyModel,
            SIGNAL(layoutChanged()),
            tvPayments,
            SLOT(resizeRowsToContents()),
            Qt::QueuedConnection);

    createColumnWidgets();
    setupWidgets();
    setupConnections();

    wPaymentSummary->hide();
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::createColumnWidgets() {
    if (!pageFields->layout() || pageFields->layout()->count() >= m_ProxyModel->columnCount()) {
        return;
    }

    QSet<int> defaultDisabledColumn;
    defaultDisabledColumn << PaymentTableModel::LastUpdate << PaymentTableModel::InitialSession
                          << PaymentTableModel::Session << PaymentTableModel::TransId;

    for (int i = 0; i < m_Model->columnCount(); i++) {
        if (m_ProxyModel->hiddenColumn(i)) {
            m_ProxyModel->showColumn(i, false);
            continue;
        }

        QCheckBox *checkBox = new QCheckBox(this);
        QString header = m_Model->headerData(i, Qt::Horizontal).toString();

        checkBox->setText(header);
        checkBox->setProperty("column", i);

        checkBox->setChecked(!defaultDisabledColumn.contains(i));
        m_ProxyModel->showColumn(i, !defaultDisabledColumn.contains(i));

        connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(showColumn(bool)));

        pageFields->layout()->addWidget(checkBox);
        m_ColumnCheckboxs.insert(i, checkBox);
    }
    pageFields->layout()->addItem(
        new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::setupWidgets() {
    m_PaymentsFilterButtonGroup = new QButtonGroup(this);
    m_PaymentsFilterButtonGroup->addButton(rbAllPayments);
    m_PaymentsFilterButtonGroup->addButton(rbPrinted);
    m_PaymentsFilterButtonGroup->addButton(rbProcessed);

    m_DateFilterButtonGroup = new QButtonGroup(this);
    m_DateFilterButtonGroup->addButton(rbAllDates);
    m_DateFilterButtonGroup->addButton(rbLastEncashment);
    m_DateFilterButtonGroup->addButton(rbDate);

    dateEdit->setDate(QDate::currentDate());

    QStringList dateRangeItems;
    dateRangeItems.insert(DayRange, tr("#day"));
    dateRangeItems.insert(WeekRange, tr("#week"));
    dateRangeItems.insert(MonthRange, tr("#month"));
    dateRangeItems.insert(ThreeMonthRange, tr("#three_month"));
    cbRange->addItems(dateRangeItems);
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::setupConnections() {
    connect(&m_PaymentTaskWatcher, SIGNAL(finished()), SLOT(onPaymentsUpdated()));
    connect(m_Model, SIGNAL(updatePayments(QString)), SLOT(onUpdatePayments(QString)));
    connect(
        m_Model, SIGNAL(showProcessWindow(bool, QString)), SLOT(onShowProcessWindow(bool, QString)));
    connect(btnPrintCurrentReceipt, SIGNAL(clicked()), SLOT(printCurrentReceipt()));
    connect(btnPrintReceipts, SIGNAL(clicked()), m_Model, SLOT(printAllReceipts()));
    connect(btnPrintFilteredReceipts, SIGNAL(clicked()), this, SLOT(printFilteredReceipts()));
    connect(btnProcessCurrentPayment, SIGNAL(clicked()), SLOT(processCurrentPayment()));
    connect(btnProcessPayments, SIGNAL(clicked()), m_Model, SLOT(processAllPayments()));

    connect(rbAllDates, SIGNAL(toggled(bool)), this, SLOT(disableDateFilter(bool)));
    connect(rbLastEncashment, SIGNAL(toggled(bool)), this, SLOT(enableLastEncashmentFilter(bool)));
    connect(rbDate, SIGNAL(toggled(bool)), this, SLOT(enableDateRangeFilter(bool)));

    connect(rbDate, SIGNAL(toggled(bool)), cbRange, SLOT(setEnabled(bool)));
    connect(rbDate, SIGNAL(toggled(bool)), dateEdit, SLOT(setEnabled(bool)));

    connect(cbRange, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDateRange()));
    connect(dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(updateDateRange()));

    connect(rbAllPayments, SIGNAL(toggled(bool)), m_ProxyModel, SLOT(disablePaymentsFilter()));
    connect(rbProcessed, SIGNAL(toggled(bool)), m_ProxyModel, SLOT(enableProcessedPaymentsFilter()));
    connect(rbPrinted, SIGNAL(toggled(bool)), m_ProxyModel, SLOT(enablePrintedPaymentsFilter()));

    connect(leSearch, SIGNAL(textChanged(QString)), m_ProxyModel, SLOT(setFilterWildcard(QString)));
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::initialize() {
    HumoServiceBackend::TAccessRights rights = m_Backend->getAccessRights();

    // Право на просмотр суммарной информации по платежам
    wPaymentSummary->setEnabled(rights.contains(HumoServiceBackend::ViewPaymentSummary));

    // Право на просмотр платежей
    tvPayments->setEnabled(rights.contains(HumoServiceBackend::ViewPayments));

    return true;
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::shutdown() {
    return true;
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::activate() {
    // Обновим состояние кнопок печати
    bool canPrint = m_Backend->getPaymentManager()->canPrint(PPSDK::CReceiptType::Payment);

    btnPrintCurrentReceipt->setEnabled(canPrint);
    btnPrintReceipts->setEnabled(canPrint);
    btnPrintFilteredReceipts->setEnabled(canPrint);

    // Обновляем все данные по платежам
    onUpdatePayments();

    connect(m_PaymentManager,
            SIGNAL(receiptPrinted(qint64, bool)),
            m_Model,
            SLOT(onReceiptPrinted(qint64, bool)));
    connect(m_PaymentManager, SIGNAL(paymentChanged(qint64)), m_Model, SLOT(onUpdatePayment(qint64)));

    QVariantMap parameters = m_Backend->getConfiguration();
    QVariantList columns = parameters[CPaymentServiceWindow::ColumnVisibility].toList();
    for (int i = 0; i < columns.size(); i++) {
        m_ColumnCheckboxs.contains(i) ? m_ColumnCheckboxs[i]->setChecked(columns[i].toBool())
                                     : m_ProxyModel->showColumn(i, columns[i].toBool());
    }

    return true;
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::deactivate() {
    disconnect(m_PaymentManager,
               SIGNAL(receiptPrinted(qint64, bool)),
               m_Model,
               SLOT(onReceiptPrinted(qint64, bool)));
    disconnect(
        m_PaymentManager, SIGNAL(paymentChanged(qint64)), m_Model, SLOT(onUpdatePayment(qint64)));

    QVariantMap parameters;
    parameters.insert(CPaymentServiceWindow::ColumnVisibility, m_ProxyModel->getColumnVisibility());
    m_Backend->setConfiguration(parameters);
    m_Backend->saveConfiguration();

    return true;
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::onUpdatePayments(const QString &aMessage) {
    QVariantMap result;

    if (m_PaymentManager->getPaymentsInfo(result)) {
        lbLastRecievedPayment->setText(result[CServiceTags::LastPaymentDate].toString());
        lbLastProcessedPayment->setText(result[CServiceTags::LastProcessedPaymentDate].toString());
        lbSuccessfulPaymentCount->setText(result[CServiceTags::SuccessfulPaymentCount].toString());
        lbFailedPaymentCount->setText(result[CServiceTags::FailedPaymentCount].toString());
    }

    aMessage.isEmpty() ? GUI::MessageBox::wait(tr("#updating_payment_data"))
                       : GUI::MessageBox::wait(aMessage);

    m_PaymentTaskWatcher.setFuture(QtConcurrent::run([this]() { loadPayments(); }));
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::onShowProcessWindow(bool aShow, const QString &aMessage) {
    if (aShow) {
        GUI::MessageBox::wait(aMessage);
    } else {
        GUI::MessageBox::hide();
    }
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::loadPayments() {
    m_PaymentInfoList = m_PaymentManager->getPayments(true);
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::onPaymentsUpdated() {
    m_Model->setPayments(m_PaymentInfoList);

    rbLastEncashment->setChecked(true);
    rbAllPayments->setChecked(true);

    GUI::MessageBox::hide(true);
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::printCurrentReceipt() {
    m_Model->printReceipt(getSelectedIndex());
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::processCurrentPayment() {
    m_Model->proccessPayment(getSelectedIndex());
}

//----------------------------------------------------------------------------
QModelIndex PaymentServiceWindow::getSelectedIndex() {
    QItem_SelectionModel *selectionModel = tvPayments->selectionModel();
    QItem_Selection item_Selection = selectionModel->selection();
    QItem_Selection sourceSelection = m_ProxyModel->mapSelectionToSource(item_Selection);

    return sourceSelection.isEmpty() ? QModelIndex() : sourceSelection.indexes().first();
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::disableDateFilter(bool aEnabled) {
    if (aEnabled) {
        m_ProxyModel->disableDateFilter();
    }
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::enableDateRangeFilter(bool aEnabled) {
    if (aEnabled) {
        updateDateRange();
    }
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::enableLastEncashmentFilter(bool aEnabled) {
    if (aEnabled) {
        QVariantMap encashmentInfo = m_Backend->getPaymentManager()->getBalanceInfo();
        if (encashmentInfo.isEmpty()) {
            rbLastEncashment->setEnabled(false);
            rbAllDates->setChecked(true);
            return;
        }

        QDateTime lastEncashment = encashmentInfo[CServiceTags::LastEncashmentDate].toDateTime();

        m_ProxyModel->setDateFilter(lastEncashment, QDateTime::currentDateTime());
    }
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::updateDateRange() {
    QDateTime end;
    end.setDate(dateEdit->date());
    end = end.addDays(1);
    QDateTime start;

    switch (cbRange->currentIndex()) {
    case DayRange:
        start = end.addDays(-1);
        break;
    case WeekRange:
        start = end.addDays(-7);
        break;
    case MonthRange:
        start = end.addMonths(-1);
        break;
    case ThreeMonthRange:
        start = end.addMonths(-3);
        break;
    default:
        start = end.addDays(-1);
    }

    m_ProxyModel->setDateFilter(start, end);
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::showColumn(bool aShow) {
    if (sender()) {
        int columnId = sender()->property("column").toInt();
        m_ProxyModel->showColumn(columnId, aShow);
    }
}

//----------------------------------------------------------------------------
PaymentTableModel::PaymentTableModel(bool aFiscalMode,
                                     PaymentManager *aPaymentManager,
                                     QObject *aParent)
    : QAbstractTableModel(aParent), m_FiscalMode(aFiscalMode), m_PaymentManager(aPaymentManager) {
    columnHeaders.insert(Id, tr("#id"));
    columnHeaders.insert(ProviderFields, tr("#provider_fields"));
    columnHeaders.insert(Amount, tr("#amount_field"));
    columnHeaders.insert(AmountAll, tr("#amount_all_field"));
    columnHeaders.insert(Provider, tr("#provider"));
    columnHeaders.insert(CreationDate, tr("#create_date_field"));
    columnHeaders.insert(LastUpdate, tr("#last_update_field"));
    columnHeaders.insert(InitialSession, tr("#initial_session"));
    columnHeaders.insert(Session, tr("#session"));
    columnHeaders.insert(TransId, tr("#trans_id"));
    columnHeaders.insert(Status, tr("#status_field"));
    columnHeaders.insert(
        Printed,
        (m_FiscalMode ? tr("#fiscal_receipt_printed_field") : tr("#receipt_printed_field")));
    columnHeaders.insert(Processed, "#processed");
}

//----------------------------------------------------------------------------
int PaymentTableModel::rowCount(const QModelIndex & /*parent*/) const {
    return m_PaymentInfoList.size();
}

//----------------------------------------------------------------------------
int PaymentTableModel::columnCount(const QModelIndex & /*parent*/) const {
    return columnHeaders.size();
}

//----------------------------------------------------------------------------
QVariant PaymentTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();

    if (role == IDRole) {
        return m_PaymentInfoList[row].getId();
    }

    if (role == DataRole) {
        switch (index.column()) {
        case Id:
            return m_PaymentInfoList[row].getId();
        case Provider:
            return m_PaymentInfoList[row].getProvider();
        case ProviderFields:
            return m_PaymentInfoList[row].getProviderFields();
        case Amount:
            return m_PaymentInfoList[row].getAmount();
        case AmountAll:
            return m_PaymentInfoList[row].getAmountAll();
        case CreationDate:
            return m_PaymentInfoList[row].getCreationDate();
        case LastUpdate:
            return m_PaymentInfoList[row].getLastUpdate();
        case InitialSession:
            return m_PaymentInfoList[row].getInitialSession();
        case Session:
            return m_PaymentInfoList[row].getSession();
        case TransId:
            return m_PaymentInfoList[row].getTransId();
        case Printed:
            return m_PaymentInfoList[row].getPrinted();
        case Status:
            return m_PaymentInfoList[row].getStatus();
        case Processed:
            return m_PaymentInfoList[row].isProcessed();
        default:
            return QVariant();
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Id:
            return m_PaymentInfoList[row].getId();
        case Provider:
            return m_PaymentInfoList[row].getProvider();
        case ProviderFields:
            return m_PaymentInfoList[row].getProviderFields();
        case Amount:
            return QString::number(m_PaymentInfoList[row].getAmount(), 'f', 2);
        case AmountAll:
            return QString::number(m_PaymentInfoList[row].getAmountAll(), 'f', 2);
        case CreationDate:
            return m_PaymentInfoList[row].getCreationDate().toString("yyyy.MM.dd hh:mm:ss");
        case LastUpdate:
            return m_PaymentInfoList[row].getLastUpdate().toString("yyyy.MM.dd hh:mm:ss:zzz");
        case InitialSession:
            return m_PaymentInfoList[row].getInitialSession();
        case Session:
            return m_PaymentInfoList[row].getSession();
        case TransId:
            return m_PaymentInfoList[row].getTransId();
        case Printed:
            return m_PaymentInfoList[row].getPrinted() ? tr("#yes") : tr("#no");
        case Status:
            return m_PaymentInfoList[row].getStatusString();
        case Processed:
            return m_PaymentInfoList[row].isProcessed();
        default:
            return QVariant();
        }
    }

    if (role == Qt::BackgroundRole) {
        auto status = m_PaymentInfoList[row].getStatus();

        // Попытка мошенничества - фиолетовая строка.
        if (status == PPSDK::EPaymentStatus::Cheated) {
            return QBrush(QColor(255, 172, 255));
        }

        // Платёж проведён, удалён или отменён - зелёная строка.
        if (status == PPSDK::EPaymentStatus::Completed ||
            status == PPSDK::EPaymentStatus::Canceled || status == PPSDK::EPaymentStatus::Deleted) {
            return QBrush(QColor(172, 255, 174));
        }

        // Платёж проводится - жёлтая строка.
        if (status == PPSDK::EPaymentStatus::ReadyForCheck) {
            return QBrush(QColor(255, 255, 172));
        }

        // Остальное - красные строки
        return QBrush(QColor(255, 172, 174));
    }

    return QVariant();
}

//----------------------------------------------------------------------------
Qt::Item_Flags PaymentTableModel::flags(const QModelIndex & /*index*/) const {
    return Qt::Item_IsSelectable | Qt::Item_IsEnabled;
}

//----------------------------------------------------------------------------
QVariant
PaymentTableModel::headerData(int aSection, Qt::Orientation aOrientation, int aRole) const {
    if ((aRole == Qt::DisplayRole) && (aOrientation == Qt::Horizontal)) {
        if (aSection >= 0 && aSection < columnHeaders.size()) {
            Column column = static_cast<Column>(aSection);
            return columnHeaders.value(column);
        } else {
            return QString();
        }
    } else {
        return QVariant();
    }
}

//----------------------------------------------------------------------------
void PaymentTableModel::printReceipt(const QModelIndex &index) {
    if (index.row() >= 0 && index.row() < m_PaymentInfoList.size()) {
        PaymentInfo payment = m_PaymentInfoList[index.row()];

        if (payment.canPrint()) {
            if (m_PaymentManager->printReceipt(payment.getId(), DSDK::EPrintingModes::None)) {
                m_PrintingQueue.insert(payment.getId());
            }

            emit showProcessWindow(true, tr("printing_receipt"));
        } else {
            GUI::MessageBox::info(tr("#printed_before"));
        }
    } else {
        GUI::MessageBox::info(tr("#select_payment_to_print"));
    }
}

//----------------------------------------------------------------------------
void PaymentTableModel::printAllReceipts() {
    foreach (const PaymentInfo &paymentInfo, m_PaymentInfoList) {
        if (paymentInfo.canPrint() && !paymentInfo.getPrinted()) {
            if (m_PaymentManager->printReceipt(paymentInfo.getId(),
                                              DSDK::EPrintingModes::Continuous)) {
                m_PrintingQueue.insert(paymentInfo.getId());
            }
        }
    }

    if (!m_PrintingQueue.isEmpty()) {
        emit showProcessWindow(true, tr("#printing %1 receipts").arg(m_PrintingQueue.size()));
    } else {
        GUI::MessageBox::info(tr("#nothing_to_print"));
    }
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::printFilteredReceipts() {
    QSet<qint64> payments;

    for (int i = 0; i < m_ProxyModel->rowCount(); ++i) {
        payments
            << m_ProxyModel->data(m_ProxyModel->index(i, 0), PaymentTableModel::IDRole).toLongLong();
    }

    m_Model->printFilteredReceipts(payments);
}

//----------------------------------------------------------------------------
void PaymentTableModel::printFilteredReceipts(const QSet<qint64> &aPaymentsID) {
    foreach (const PaymentInfo &paymentInfo, m_PaymentInfoList) {
        if (paymentInfo.canPrint() && !paymentInfo.getPrinted() &&
            aPaymentsID.contains(paymentInfo.getId())) {
            if (m_PaymentManager->printReceipt(paymentInfo.getId(),
                                              DSDK::EPrintingModes::Continuous)) {
                m_PrintingQueue.insert(paymentInfo.getId());
            }
        }
    }

    if (!m_PrintingQueue.isEmpty()) {
        emit showProcessWindow(true, tr("#printing %1 receipts").arg(m_PrintingQueue.size()));
    } else {
        GUI::MessageBox::info(tr("#nothing_to_print"));
    }
}

//----------------------------------------------------------------------------
void PaymentTableModel::onReceiptPrinted(qint64 aPaymentId, bool aErrorHappened) {
    if (aErrorHappened) {
        GUI::MessageBox::critical(tr("#error_occurred_printing"));
        m_PrintingQueue.clear();
        return;
    }

    if (m_PrintingQueue.isEmpty()) {
        return;
    }

    m_PrintingQueue.remove(aPaymentId);

    emit layoutAboutToBeChanged();

    int row = m_PaymentRowIndex[aPaymentId];
    m_PaymentInfoList[row].setPrinted(true);

    emit dataChanged(index(row, 0), index(row, columnCount()));
    emit layoutChanged();

    if (m_PrintingQueue.isEmpty()) {
        if (!aErrorHappened) {
            emit showProcessWindow(false, "");
        }
    } else {
        emit showProcessWindow(true, tr("#printing %1 receipts").arg(m_PrintingQueue.size()));
    }
}

//----------------------------------------------------------------------------
void PaymentTableModel::onUpdatePayment(qint64 aPaymentId) {
    if (m_PaymentRowIndex.contains(aPaymentId)) {
        emit layoutAboutToBeChanged();
        int row = m_PaymentRowIndex[aPaymentId];

        m_PaymentInfoList[row] = m_PaymentManager->getPayment(aPaymentId);

        emit dataChanged(index(row, 0), index(row, columnCount()));
        emit layoutChanged();
    }
}

//----------------------------------------------------------------------------
void PaymentTableModel::proccessPayment(const QModelIndex &index) {
    if (index.row() >= 0 && index.row() < m_PaymentInfoList.size()) {
        PaymentInfo payment = m_PaymentInfoList[index.row()];

        if (payment.canProcess()) {
            m_PaymentManager->processPayment(payment.getId());
            onUpdatePayment(payment.getId());

            GUI::MessageBox::info(tr("#process"));
        } else {
            GUI::MessageBox::info(tr("#bad_status"));
        }
    } else {
        GUI::MessageBox::info(tr("#select_payment_to_process"));
    }
}

//----------------------------------------------------------------------------
void PaymentTableModel::proccessNextPayment() {
    if (m_ProcessPayments.payments.count()) {
        auto payment = m_ProcessPayments.payments.takeFirst();

        if (payment.canProcess()) {
            m_PaymentManager->processPayment(payment.getId());

            onUpdatePayment(payment.getId());
            ++m_ProcessPayments.processed;
        }
    } else {
        if (m_ProcessPayments.processed) {
            GUI::MessageBox::info(tr("#process %1 payments").arg(m_ProcessPayments.processed));
        } else {
            GUI::MessageBox::info(tr("#nothing_to_process"));
        }

        m_ProcessPayments.clear();
        return;
    }

    // Вызываем обработку следующего платежа
    QMetaObject::invokeMethod(this, "proccessNextPayment", Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
void PaymentTableModel::processAllPayments() {
    m_ProcessPayments.payments = m_PaymentInfoList;
    m_ProcessPayments.processed = 0;

    GUI::MessageBox::wait(tr("#updating_payment_data"), true);
    GUI::MessageBox::subscribe(this);

    // Вызываем обработку следующего платежа
    QMetaObject::invokeMethod(this, "proccessNextPayment", Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
void PaymentTableModel::onClicked(const QVariantMap &) {
    // прерываем обработку платежей
    m_ProcessPayments.payments.clear();
}

//----------------------------------------------------------------------------
void PaymentTableModel::setPayments(QList<PaymentInfo> aPaymentInfoList) {
    beginResetModel();

    m_ProcessPayments.clear();

    m_PaymentInfoList = aPaymentInfoList;

    m_PaymentRowIndex.clear();
    for (int i = 0; i < m_PaymentInfoList.size(); ++i) {
        m_PaymentRowIndex.insert(m_PaymentInfoList[i].getId(), i);
    }

    endResetModel();
}

//----------------------------------------------------------------------------
PaymentProxyModel::PaymentProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_PaymentFilter(AllPayments), m_DateFilterEnabled(false) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortRole(PaymentTableModel::DataRole);
}

//----------------------------------------------------------------------------
void PaymentProxyModel::showColumn(int aColumn, bool aShow) {
    PaymentTableModel::Column column = static_cast<PaymentTableModel::Column>(aColumn);
    m_Columns[column] = aShow;
    // QSettings settings;
    // settings.setValue(QString("ServiceMenu/column%1").arg(aColumn), aShow);
    invalidateFilter();
}

//----------------------------------------------------------------------------
QVariantList PaymentProxyModel::getColumnVisibility() const {
    QVariantList columns;
    for (int i = 0; i < m_Columns.size(); i++)
        columns << m_Columns[i];
    return columns;
}

//----------------------------------------------------------------------------
bool PaymentProxyModel::hiddenColumn(int aColumn) const {
    switch (aColumn) {
    case PaymentTableModel::Id:
    case PaymentTableModel::Processed:
        return true;
    default:
        return false;
    }
}

//----------------------------------------------------------------------------
void PaymentProxyModel::disableDateFilter() {
    m_DateFilterEnabled = false;
    invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::setDateFilter(const QDateTime &aFrom, const QDateTime &aTo) {
    m_StartDateTime = aFrom;
    m_EndDateTime = aTo;
    m_DateFilterEnabled = true;

    invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::disablePaymentsFilter() {
    m_PaymentFilter = AllPayments;
    invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::enablePrintedPaymentsFilter() {
    m_PaymentFilter = PrintedPayments;
    invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::enableProcessedPaymentsFilter() {
    m_PaymentFilter = ProcessedPayments;
    invalidateFilter();
}

//----------------------------------------------------------------------------
bool PaymentProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    QAbstractItem_Model *sourceModel = this->sourceModel();
    QRegularExpression regExp = filterRegularExpression();

    QModelIndex providerFieldsIndex =
        sourceModel->index(sourceRow, PaymentTableModel::ProviderFields, sourceParent);
    QString providerFieldsValue =
        sourceModel->data(providerFieldsIndex, PaymentTableModel::DataRole).toString();
    bool providerFieldsFilter = providerFieldsValue.contains(regExp);

    QModelIndex initialSessionIndex =
        sourceModel->index(sourceRow, PaymentTableModel::InitialSession, sourceParent);
    QString initialSessionValue =
        sourceModel->data(initialSessionIndex, PaymentTableModel::DataRole).toString();
    bool initialSessionFilter = initialSessionValue.contains(regExp);

    QModelIndex sessionIndex =
        sourceModel->index(sourceRow, PaymentTableModel::Session, sourceParent);
    QString sessionValue = sourceModel->data(sessionIndex, PaymentTableModel::DataRole).toString();
    bool sessionFilter = sessionValue.contains(regExp);

    QModelIndex transIdIndex =
        sourceModel->index(sourceRow, PaymentTableModel::TransId, sourceParent);
    QString transIdValue = sourceModel->data(transIdIndex, PaymentTableModel::DataRole).toString();
    bool transIdFilter = transIdValue.contains(regExp);

    bool dateFilter = true;
    if (m_DateFilterEnabled) {
        QModelIndex creationDateIndex =
            sourceModel->index(sourceRow, PaymentTableModel::CreationDate, sourceParent);
        QDateTime creationDateTimeValue =
            sourceModel->data(creationDateIndex, PaymentTableModel::DataRole).toDateTime();
        dateFilter =
            creationDateTimeValue >= m_StartDateTime && creationDateTimeValue <= m_EndDateTime;
    }

    bool processedFilter = true;
    if (m_PaymentFilter == ProcessedPayments) {
        QModelIndex processedIndex =
            sourceModel->index(sourceRow, PaymentTableModel::Processed, sourceParent);
        processedFilter = !sourceModel->data(processedIndex, PaymentTableModel::DataRole).toBool();
    }

    bool printedFilter = true;
    if (m_PaymentFilter == PrintedPayments) {
        QModelIndex printedIndex =
            sourceModel->index(sourceRow, PaymentTableModel::Printed, sourceParent);
        printedFilter = !sourceModel->data(printedIndex, PaymentTableModel::DataRole).toBool();
    }

    return (providerFieldsFilter || initialSessionFilter || sessionFilter || transIdFilter) &&
           dateFilter && processedFilter && printedFilter;
}

//----------------------------------------------------------------------------
bool PaymentProxyModel::filterAcceptsColumn(int sourceColumn,
                                            const QModelIndex &sourceParent) const {
    Q_UNUSED(sourceParent);

    PaymentTableModel::Column column = static_cast<PaymentTableModel::Column>(sourceColumn);
    return m_Columns.value(column, true);
}

//----------------------------------------------------------------------------
