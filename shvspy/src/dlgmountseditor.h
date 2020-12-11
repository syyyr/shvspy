#ifndef DLGMOUNTSSEDITOR_H
#define DLGMOUNTSEDITOR_H

#include <QDialog>
#include <QMap>

#include "shv/chainpack/rpcvalue.h"

#include <shv/iotqt/rpc/rpcresponsecallback.h>
#include <shv/iotqt/rpc/clientconnection.h>

namespace Ui {
class DlgMountsEditor;
}

class DlgMountsEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DlgMountsEditor(QWidget *parent, shv::iotqt::rpc::ClientConnection *rpc_connection);
	~DlgMountsEditor();
	void init(const std::string &acl_node_path);

private:
	std::string aclEtcMountsNodePath();
	//std::string aclEtcAccessNodePath();

	QString selectedMount();
	void listMounts();
	void getMountPoint(const QString &id);
	void getMountDescription(const QString &id);
	//void callDeleteAccessForMount(const std::string &mount);

	void onAddMountClicked();
	void onDeleteMountClicked();
	void onEditMountClicked();
	void onTableMountDoubleClicked(QModelIndex ix);
	void onRpcCallsFinished();
	void setFilter(const QString &filter);

	void setStatusText(const QString &txt);
private:
	struct MountPointInfo {
		QString id;
		QString mountPoint;
		QString description;
	};
	Ui::DlgMountsEditor *ui;
	std::string m_aclEtcNodePath;
	shv::iotqt::rpc::ClientConnection *m_rpcConnection = nullptr;
	QMap<QString, MountPointInfo> m_mountPoints;
	bool m_rpcCallFailed;
	int m_rpcCallsToComplete;
	QString m_lastCurrentId;
};

#endif // DLGMOUNTSEDITOR_H
