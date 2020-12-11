#include "dlgroleseditor.h"
#include "ui_dlgroleseditor.h"

#include "dlgaddeditrole.h"
#include "shv/core/assert.h"
#include <QMessageBox>

static const std::string VALUE_METHOD = "value";
static const std::string SET_VALUE_METHOD = "setValue";

DlgRolesEditor::DlgRolesEditor(QWidget *parent, shv::iotqt::rpc::ClientConnection *rpc_connection) :
	QDialog(parent),
	ui(new Ui::DlgRolesEditor)
{
	ui->setupUi(this);

	SHV_ASSERT_EX(rpc_connection != nullptr, "RPC connection is NULL");

	m_rpcConnection = rpc_connection;

	static constexpr double ROW_HEIGHT_RATIO = 1.3;
	static QStringList INFO_HEADER_NAMES {{ tr("Role") }};

	ui->twRoles->setColumnCount(INFO_HEADER_NAMES.count());
	ui->twRoles->setHorizontalHeaderLabels(INFO_HEADER_NAMES);
	ui->twRoles->horizontalHeader()->setStretchLastSection(true);
	ui->twRoles->verticalHeader()->setDefaultSectionSize(ui->twRoles->fontMetrics().height() * ROW_HEIGHT_RATIO);
	ui->twRoles->verticalHeader()->setVisible(false);

	connect(ui->pbAddRole, &QPushButton::clicked, this, &DlgRolesEditor::onAddRoleClicked);
	connect(ui->pbDeleteRole, &QPushButton::clicked, this, &DlgRolesEditor::onDeleteRoleClicked);
	connect(ui->pbEditRole, &QPushButton::clicked, this, &DlgRolesEditor::onEditRoleClicked);
	connect(ui->twRoles, &QTableWidget::doubleClicked, this, &DlgRolesEditor::onTableRoleDoubleClicked);
	connect(ui->leFilter, &QLineEdit::textChanged, this, &DlgRolesEditor::setFilter);

	setStatusText(QString());
}

DlgRolesEditor::~DlgRolesEditor()
{
	delete ui;
}

void DlgRolesEditor::init(const std::string &acl_node_path)
{
	m_aclEtcNodePath = acl_node_path;
	listRoles();
}

std::string DlgRolesEditor::aclEtcRolesNodePath()
{
	return m_aclEtcNodePath + "/roles";
}

std::string DlgRolesEditor::aclEtcAccessNodePath()
{
	return m_aclEtcNodePath + "/access";
}

QString DlgRolesEditor::selectedRole()
{
	return (ui->twRoles->currentIndex().isValid()) ? ui->twRoles->currentItem()->text() : QString();
}

void DlgRolesEditor::onAddRoleClicked()
{
	DlgAddEditRole dlg(this, m_rpcConnection, m_aclEtcNodePath, DlgAddEditRole::DialogType::Add);
	if (dlg.exec() == QDialog::Accepted){
		listRoles();
	}
}

void DlgRolesEditor::onDeleteRoleClicked()
{
	std::string role = selectedRole().toStdString();

	if (role.empty()){
		setStatusText(tr("Select role in the table above."));
		return;
	}

	setStatusText(QString());

	if (QMessageBox::question(this, tr("Delete role"), tr("Do you really want to delete data and associated access pahts for role") + " " + QString::fromStdString(role) + "?") == QMessageBox::Yes){
		int rqid = m_rpcConnection->nextRequestId();
		shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(m_rpcConnection, rqid, this);

		cb->start(this, [this, role](const shv::chainpack::RpcResponse &response) {
			if(response.isValid()){
				if(response.isError()) {
					setStatusText(tr("Failed to delete role.") + " " + QString::fromStdString(response.error().toString()));
				}
				else{
					callDeleteAccessForRole(role);
					listRoles();
					setStatusText(QString());
				}
			}
			else{
				setStatusText(tr("Request timeout expired"));
			}
		});

		shv::chainpack::RpcValue::List params{shv::chainpack::RpcValue::String(role), {}};
		m_rpcConnection->callShvMethod(rqid, aclEtcRolesNodePath(), SET_VALUE_METHOD, params);
	}
}

void DlgRolesEditor::onEditRoleClicked()
{
	QString role = selectedRole();

	if (role.isEmpty()){
		setStatusText(tr("Select role in the table above."));
		return;
	}

	setStatusText(QString());

	DlgAddEditRole dlg(this, m_rpcConnection, m_aclEtcNodePath, DlgAddEditRole::DialogType::Edit);
	dlg.init(role);

	if (dlg.exec() == QDialog::Accepted){
		listRoles();
	}
}

void DlgRolesEditor::onTableRoleDoubleClicked(QModelIndex ix)
{
	Q_UNUSED(ix);
	onEditRoleClicked();
}

void DlgRolesEditor::listRoles()
{
	if (m_rpcConnection == nullptr)
		return;

	for (int i = 0; i < ui->twRoles->rowCount(); ++i) {
		ui->twRoles->takeItem(i, 0);
	}
	qDeleteAll(m_tableRows);
	m_tableRows.clear();

	ui->twRoles->clearContents();
	ui->twRoles->setRowCount(0);

	int rqid = m_rpcConnection->nextRequestId();
	shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(m_rpcConnection, rqid, this);

	cb->start(this, [this](const shv::chainpack::RpcResponse &response) {
		if(response.isValid()){
			if(response.isError()) {
				setStatusText(tr("Failed to load roles.") + " " + QString::fromStdString(response.error().toString()));
			}
			else{
				if (response.result().isList()){
					shv::chainpack::RpcValue::List res = response.result().toList();

					for (size_t i = 0; i < res.size(); i++){
						QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(res.at(i).toStdString()));
						item->setFlags(item->flags() & ~Qt::ItemIsEditable);
						m_tableRows << item;
					}
					setFilter(ui->leFilter->text());
				}
				setStatusText(QString());
			}
		}
		else{
			setStatusText(tr("Request timeout expired"));
		}
	});

	m_rpcConnection->callShvMethod(rqid, aclEtcRolesNodePath(), shv::chainpack::Rpc::METH_LS);
}

void DlgRolesEditor::callDeleteAccessForRole(const std::string &role)
{
	if (m_rpcConnection == nullptr)
		return;

	setStatusText(tr("Deleting access paths for role:") + " " + QString::fromStdString(role));

	int rqid = m_rpcConnection->nextRequestId();
	shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(m_rpcConnection, rqid, this);

	cb->start(this, [this](const shv::chainpack::RpcResponse &response) {
		if (response.isValid()){
			if(response.isError()) {
				setStatusText(tr("Failed to delete access pahts.") + " " + QString::fromStdString(response.error().toString()));
			}
			else{
				setStatusText(QString());
			}
		}
		else{
			setStatusText(tr("Request timeout expired"));
		}
	});

	shv::chainpack::RpcValue::List params{shv::chainpack::RpcValue::String(role), {}};
	m_rpcConnection->callShvMethod(rqid, aclEtcAccessNodePath(), SET_VALUE_METHOD, params);
}

void DlgRolesEditor::setStatusText(const QString &txt)
{
	if(txt.isEmpty()) {
		ui->lblStatus->hide();
	}
	else {
		ui->lblStatus->show();
		ui->lblStatus->setText(txt);
	}
}

void DlgRolesEditor::setFilter(const QString &filter)
{
	QString l_filter = filter.toLower().trimmed();
	for (int i = 0; i < ui->twRoles->rowCount(); ++i) {
		ui->twRoles->takeItem(i, 0);
	}
	int j = 0;
	for (QTableWidgetItem *item : m_tableRows) {
		if (item->text().toLower().contains(l_filter)) {
			if (ui->twRoles->rowCount() <= j) {
				ui->twRoles->insertRow(j);
			}
			ui->twRoles->setItem(j, 0, item);
			++j;
		}
	}
	while (j < ui->twRoles->rowCount()) {
		ui->twRoles->removeRow(j);
	}
}
