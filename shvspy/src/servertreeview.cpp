#include "servertreeview.h"

#include <QKeyEvent>

ServerTreeView::ServerTreeView(QWidget *parent)
	: Super(parent)
{
	grabGesture(Qt::GestureType::TapAndHoldGesture);
	grabGesture(Qt::GestureType::TapGesture);
}

void ServerTreeView::keyPressEvent(QKeyEvent *ev)
{
	if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) {
		emit enterKeyPressed(currentIndex());
		ev->accept();
		return;
	}
	Super::keyPressEvent(ev);
}

