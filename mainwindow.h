#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTextEdit>
#include <map>
#include <string>
#include "tcpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
namespace Ui {
class Dialog;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *, QEvent *);
    void add_key_dialog(std::string key, std::string dialog);
    void add_key_name(std::string key, std::string name);
    void addItem(QListWidgetItem *item);
    QListWidget *get_list();
    std::map<std::string, std::string> &get_history();
signals:
    void button_send(std::string id, std::string text);
    void button_close_addition(std::string name,
                               std::string public_key,
                               std::string ip_address);
private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    std::map<std::string, std::string> get_dialog_by_key;
    std::map<std::string, std::string> get_name_by_key;
    std::map<std::string, std::string> id_to_history;
};

class MyTextEdit : public QTextEdit {
    Q_OBJECT
public:
    void keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Return) {
            qInfo() << "hey";
        } else {
            QTextEdit::keyPressEvent(event);
        }
    }
};

class Dialog : public QDialog {
    Q_OBJECT

public:
    Dialog();
    Dialog(MainWindow *);
    virtual ~Dialog() {
    }
    // public slots:

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::Dialog *ui;
    MainWindow *m;
};

#endif  // MAINWINDOW_H

// void Dialog::on_buttonBox_accepted()
//{

//}
