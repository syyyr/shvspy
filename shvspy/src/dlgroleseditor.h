#ifndef DLGROLESSEDITOR_H
#define DLGROLESEDITOR_H

#include <QDialog>

#include "shv/chainpack/rpcvalue.h"

#include <shv/iotqt/rpc/rpcresponsecallback.h>
#include <shv/iotqt/rpc/clientconnection.h>

namespace Ui {
class DlgRolesEditor;
}

class QTableWidgetItem;

class DlgRolesEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DlgRolesEditor(QWidget *parent, shv::iotqt::rpc::ClientConnection *rpc_connection);
	~DlgRolesEditor();
	void init(const std::string &acl_node_path);

private:
	std::string aclEtcRolesNodePath();
	std::string aclEtcAccessNodePath();

	QString selectedRole();
	void listRoles();
	void callDeleteAccessForRole(const std::string &role);

	void onAddRoleClicked();
	void onDeleteRoleClicked();
	void onEditRoleClicked();
	void onTableRoleDoubleClicked(QModelIndex ix);

	void setStatusText(const QString &txt);
	void setFilter(const QString &filter);

private:
	Ui::DlgRolesEditor *ui;
	std::string m_aclEtcNodePath;
	shv::iotqt::rpc::ClientConnection *m_rpcConnection = nullptr;
	QVector<QTableWidgetItem*> m_tableRows;
};

#endif // DLGROLESEDITOR_H
