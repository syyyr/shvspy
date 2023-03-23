#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

namespace Ui {
class MainWindow;
}
class QCloseEvent;
class QStandardItemModel;

class ShvBrokerNodeItem;

namespace shv::chainpack { class RpcValue; }

class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	typedef QMainWindow Super;
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;
protected:
	Q_SLOT void on_actAddServer_triggered();
	Q_SLOT void on_actEditServer_triggered();
	Q_SLOT void on_actCopyServer_triggered();
	Q_SLOT void on_actRemoveServer_triggered();
	Q_SLOT void on_actHelpAbout_triggered();

	Q_SLOT void on_treeServers_doubleClicked(const QModelIndex &ix) {openNode(ix);}
	Q_SLOT void on_treeServers_enterKeyPressed(const QModelIndex &ix) {openNode(ix);}
	Q_SLOT void on_treeServers_customContextMenuRequested(const QPoint &pos);
	Q_SLOT void onShvTreeViewCurrentSelectionChanged(const QModelIndex &curr_ix, const QModelIndex &prev_ix);

	void openLogInspector();

	void resizeAttributesViewSectionsToFit();

	void editServer(ShvBrokerNodeItem *srv, bool copy_server);
	void openNode(const QModelIndex &ix);
	void displayResult(const QModelIndex &ix);
	void displayValue(const shv::chainpack::RpcValue &rv);
	void editMethodParameters(const QModelIndex &ix);
	void editStringParameter(const QModelIndex &ix);
	void editCponParameters(const QModelIndex &ix);

	void onAttributesTableContextMenu(const QPoint &point);
	void onNotificationsDoubleClicked(const QModelIndex &ix);

	void closeEvent(QCloseEvent *ev) Q_DECL_OVERRIDE;
	bool event(QEvent* event) Q_DECL_OVERRIDE;
	void saveSettings();
private:
	void checkSettingsReady();

private:
	Ui::MainWindow *ui;
	QStandardItemModel *m_opcObjects;
	QSettings m_settings;
};

#endif // MAINWINDOW_H
