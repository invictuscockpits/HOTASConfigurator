#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QElapsedTimer>
#include <QWidget>

namespace Ui {
class DebugWindow;
}

class DebugWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = nullptr);
    ~DebugWindow();

    void retranslateUi();

    void devicePacketReceived();
    void resetPacketsCount();

    void logicalButtonState(int buttonNumber, bool state);

    Q_INVOKABLE // Required for multithreaded message handler (see CustomMessageHandler in main.cpp)
        void printMsg(const QString &msg);

private slots:
    void on_checkBox_WriteLog_clicked(bool checked);
    void on_pushButton_ClearAppLog_clicked();
    void on_pushButton_ClearButtonLog_clicked();

private:
    Ui::DebugWindow *ui;
    void buttonLogReset();

    int m_packetsCount;
    QElapsedTimer m_timer;
    bool m_writeToFile;
};

#endif // DEBUGWINDOW_H
