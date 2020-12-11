#include "dlgmountseditor.h"
#include "ui_dlgmountseditor.h"

#include "dlgaddeditmount.h"
#include "shv/core/assert.h"
#include <QMessageBox>

static const std::string VALUE_METHOD = "value";
static const std::string SET_VALUE_METHOD = "setValue";

DlgMountsEditor::DlgMountsEditor(QWidget *parent, shv::iotqt::rpc::ClientConnection *rpc_connection) :
	QDialog(parent),
	ui(new Ui::DlgMountsEditor)
{
	ui->setupUi(this);

	SHV_ASSERT_EX(rpc_connection != nullptr, "RPC connection is NULL");

	m_rpcConnection = rpc_connection;

	static constexpr double ROW_HEIGHT_RATIO = 1.3;
	static QStringList INFO_HEADER_NAMES { tr("Device ID"), tr("Mount point"), tr("Description") };

	ui->twMounts->setColumnCount(INFO_HEADER_NAMES.count());
	ui->twMounts->setHorizontalHeaderLabels(INFO_HEADER_NAMES);
	ui->twMounts->horizontalHeader()->setStretchLastSection(true);
	ui->twMounts->verticalHeader()->setDefaultSectionSize(ui->twMounts->fontMetrics().height() * ROW_HEIGHT_RATIO);
	ui->twMounts->verticalHeader()->setVisible(false);
	ui->twMounts->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(ui->pbAddMount, &QPushButton::clicked, this, &DlgMountsEditor::onAddMountClicked);
	connect(ui->pbDeleteMount, &QPushButton::clicked, this, &DlgMountsEditor::onDeleteMountClicked);
	connect(ui->pbEditMount, &QPushButton::clicked, this, &DlgMountsEditor::onEditMountClicked);
	connect(ui->twMounts, &QTableWidget::doubleClicked, this, &DlgMountsEditor::onTableMountDoubleClicked);
	connect(ui->leFilter, &QLineEdit::textChanged, this, &DlgMountsEditor::setFilter);

	setStatusText(QString());
}

DlgMountsEditor::~DlgMountsEditor()
{
	delete ui;
}

void DlgMountsEditor::init(const std::string &acl_node_path)
{
	m_aclEtcNodePath = acl_node_path;
	listMounts();
}

std::string DlgMountsEditor::aclEtcMountsNodePath()
{
	return m_aclEtcNodePath + "/mounts";
}

//std::string DlgMountsEditor::aclEtcAccessNodePath()
//{
//	return m_aclEtcNodePath + "/access";
//}

QString DlgMountsEditor::selectedMount()
{
	return ui->twMounts->currentRow() != -1 ? ui->twMounts->item(ui->twMounts->currentRow(), 0)->text() : QString();
}

void DlgMountsEditor::onAddMountClicked()
{
	DlgAddEditMount dlg(this, m_rpcConnection, m_aclEtcNodePath, DlgAddEditMount::DialogType::Add);
	if (dlg.exec() == QDialog::Accepted){
		listMounts();
	}
}

void DlgMountsEditor::onDeleteMountClicked()
{
	std::string mount = selectedMount().toStdString();

	if (mount.empty()){
		setStatusText(tr("Select device id in the table above."));
		return;
	}

	setStatusText(QString());

	if (QMessageBox::question(this, tr("Delete mount"), tr("Do you really want to delete mount definition") + " " + QString::fromStdString(mount) + "?") == QMessageBox::Yes){
		int rqid = m_rpcConnection->nextRequestId();
		shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(m_rpcConnection, rqid, this);

		cb->start(this, [this, mount](const shv::chainpack::RpcResponse &response) {
			if(response.isValid()){
				if(response.isError()) {
					setStatusText(tr("Failed to delete mount definition.") + " " + QString::fromStdString(response.error().toString()));
				}
				else {
					listMounts();
				}
			}
			else{
				setStatusText(tr("Request timeout expired"));
			}
		});

		shv::chainpack::RpcValue::List params{shv::chainpack::RpcValue::String(mount), {}};
		m_rpcConnection->callShvMethod(rqid, aclEtcMountsNodePath(), SET_VALUE_METHOD, params);
	}
}

void DlgMountsEditor::onEditMountClicked()
{
	QString mount = selectedMount();

	if (mount.isEmpty()){
		setStatusText(tr("Select device id in the table above."));
		return;
	}

	setStatusText(QString());

	DlgAddEditMount dlg(this, m_rpcConnection, m_aclEtcNodePath, DlgAddEditMount::DialogType::Edit);
	dlg.init(mount);

	if (dlg.exec() == QDialog::Accepted){
		listMounts();
	}
}

void DlgMountsEditor::onTableMountDoubleClicked(QModelIndex ix)
{
	Q_UNUSED(ix);
	onEditMountClicked();
}

void DlgMountsEditor::onRpcCallsFinished()
{
	ui->pbAddMount->setEnabled(true);
	ui->pbEditMount->setEnabled(true);
	ui->pbDeleteMount->setEnabled(true);
	ui->leFilter->setEnabled(true);
	setStatusText(QString());
	setFilter(ui->leFilter->text());
	if (!m_lastCurrentId.isEmpty()) {
		for (int i = 0; i < ui->twMounts->rowCount(); ++i) {
			if (ui->twMounts->item(i, 0)->text() == m_lastCurrentId) {
				ui->twMounts->setCurrentCell(i, 0);
				break;
			}
		}
	}
}

void DlgMountsEditor::setFilter(const QString &filter)
{
	QString l_filter = filter.toLower().trimmed();
	ui->twMounts->clearContents();
	ui->twMounts->setRowCount(0);

	int i = 0;
	for (const MountPointInfo &info : m_mountPoints) {
		if (info.id.toLower().contains(l_filter) || info.mountPoint.toLower().contains(l_filter) || info.description.toLower().contains(l_filter)) {
			ui->twMounts->insertRow(i);
			QTableWidgetItem *id_item = new QTableWidgetItem(info.id);
			id_item->setFlags(id_item->flags() & ~Qt::ItemIsEditable);
			ui->twMounts->setItem(i, 0, id_item);

			QTableWidgetItem *mountpoint_item = new QTableWidgetItem(info.mountPoint);
			mountpoint_item->setFlags(mountpoint_item->flags() & ~Qt::ItemIsEditable);
			ui->twMounts->setItem(i, 1, mountpoint_item);

			QTableWidgetItem *description_item = new QTableWidgetItem(info.description);
			description_item->setFlags(description_item->flags() & ~Qt::ItemIsEditable);
			ui->twMounts->setItem(i, 2, description_item);

			++i;
		}
	}
}

void DlgMountsEditor::listMounts()
{
	if (m_rpcConnection == nullptr)
		return;

	m_lastCurrentId = ui->twMounts->currentRow() == -1 ? QString() : ui->twMounts->item(ui->twMounts->currentRow(), 0)->text();

	ui->twMounts->clearContents();
	ui->twMounts->setRowCount(0);
	m_mountPoints.clear();

	ui->pbAddMount->setEnabled(false);
	ui->pbEditMount->setEnabled(false);
	ui->pbDeleteMount->setEnabled(false);
	ui->leFilter->setEnabled(false);
	setStatusText(tr("Loading..."));

	int rqid = m_rpcConnection->nextRequestId();
	shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(m_rpcConnection, rqid, this);

	cb->start(this, [this](const shv::chainpack::RpcResponse &response) {
		if(response.isValid()){
			if(response.isError()) {
				setStatusText(tr("Failed to load mount definition.") + " " + QString::fromStdString(response.error().toString()));
			}
			else{
				if (response.result().isList()){
					shv::chainpack::RpcValue::List res = response.result().toList();

					m_rpcCallsToComplete = res.size() * 2;
					m_rpcCallFailed = false;
					for (size_t i = 0; i < res.size(); i++){
						QString id = QString::fromStdString(res.at(i).toString());
						m_mountPoints[id].id = id;
						getMountPoint(id);
						getMountDescription(id);
					}
				}
			}
		}
		else{
			setStatusText(tr("Request timeout expired"));
		}
	});

	m_rpcConnection->callShvMethod(rqid, aclEtcMountsNodePath(), shv::chainpack::Rpc::METH_LS);
}

void DlgMountsEditor::getMountPoint(const QString &id)
{
	if (m_rpcConnection == nullptr)
		return;

	int rqid = m_rpcConnection->nextRequestId();
	shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(m_rpcConnection, rqid, this);

	cb->start(this, [this, id](const shv::chainpack::RpcResponse &response) {
		if (m_rpcCallFailed) {
			return;
		}
		if (!response.isSuccess()) {
			setStatusText(tr("Failed to load mountpoint definition.") + " " + QString::fromStdString(response.error().toString()));
			m_rpcCallFailed = true;
		}
		else {
			if (response.result().isString()){
				m_mountPoints[id].mountPoint = QString::fromStdString(response.result().toString());
			}
			if (--m_rpcCallsToComplete == 0) {
				onRpcCallsFinished();
			}
		}
	});

	m_rpcConnection->callShvMethod(rqid, aclEtcMountsNodePath() + "/" + id.toStdString() + "/mountPoint", shv::chainpack::Rpc::METH_GET);

}

void DlgMountsEditor::getMountDescription(const QString &id)
{
	if (m_rpcConnection == nullptr)
		return;

	int rqid = m_rpcConnection->nextRequestId();
	shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(m_rpcConnection, rqid, this);

	cb->start(this, [this, id](const shv::chainpack::RpcResponse &response) {
		if (m_rpcCallFailed) {
			return;
		}
		if (!response.isSuccess()) {
			setStatusText(tr("Failed to load mountpoint definition.") + " " + QString::fromStdString(response.error().toString()));
			m_rpcCallFailed = true;
		}
		else {
			if (response.result().isString()){
				m_mountPoints[id].description = QString::fromStdString(response.result().toString());
			}
			if (--m_rpcCallsToComplete == 0) {
				onRpcCallsFinished();
			}
		}
	});

	m_rpcConnection->callShvMethod(rqid, aclEtcMountsNodePath() + "/" + id.toStdString() + "/description", shv::chainpack::Rpc::METH_GET);
}

void DlgMountsEditor::setStatusText(const QString &txt)
{
	if(txt.isEmpty()) {
		ui->lblStatus->hide();
	}
	else {
		ui->lblStatus->show();
		ui->lblStatus->setText(txt);
	}
}

