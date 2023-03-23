#ifndef SERVERTREEVIEW_H
#define SERVERTREEVIEW_H

#include <QGestureEvent>
#include <QTreeView>
#include <QTouchEvent>
#include <QEvent>
#include <android/log.h>
#define  LOG_TAG "shvspy-log-output-vole-pico"
#define  ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

class ServerTreeView : public QTreeView
{
	Q_OBJECT
private:
	typedef QTreeView Super;
public:
	ServerTreeView(QWidget *parent);

protected:
	Q_SIGNAL void enterKeyPressed(const QModelIndex &ix);
	void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
	bool event(QEvent* event) Q_DECL_OVERRIDE {
		switch (event->type()) {
		case QEvent::Gesture: {
			if (auto gesture = static_cast<QGestureEvent*>(event)->gesture(Qt::GestureType::TapAndHoldGesture)) {
				auto tap_n_hold = static_cast<QTapAndHoldGesture*>(gesture);
				if (tap_n_hold->state() == Qt::GestureState::GestureStarted) {
					ALOG("tap_n_hold gesture started");
					return true;
				}

				if (tap_n_hold->state() == Qt::GestureState::GestureFinished) {
					auto point = tap_n_hold->position().toPoint();
					
					ALOG("tap_n_hold gesture finished x %d y %d current: %d", point.x(), point.y(), are_we_tapping_the_current_element_question_mark(tap_n_hold));
					customContextMenuRequested(mapFromGlobal(point));
					m_gotTapAndHoldWhileTapping = true;
					return true;
				}
			}
			if (auto gesture = static_cast<QGestureEvent*>(event)->gesture(Qt::GestureType::TapGesture)) {
				auto tap = static_cast<QTapGesture*>(gesture);
				if (tap->state() == Qt::GestureState::GestureStarted) {
					ALOG("tap gesture started current element");
					m_gotTapAndHoldWhileTapping = false;
					return true;
				}
				if (tap->state() == Qt::GestureState::GestureFinished && are_we_tapping_the_current_element_question_mark(tap)) {
					ALOG("tap gesture finished current element");
					if (!m_gotTapAndHoldWhileTapping) {
						emit doubleClicked(indexAt(tap->position().toPoint()));
					}
					return true;
				}
			}
			break;
		}

		default:
			break;
		}
		return Super::event(event);
	}

private:
	bool are_we_tapping_the_current_element_question_mark(QTapAndHoldGesture* event) {
		return selectedIndexes().contains(indexAt(event->position().toPoint()));
	};
	bool are_we_tapping_the_current_element_question_mark(QTapGesture* event) {
		return selectedIndexes().contains(indexAt(event->position().toPoint()));
	};

	bool m_gotTapAndHoldWhileTapping = false;

};

#endif // SERVERTREEVIEW_H
