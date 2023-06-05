#include "mainwindow.h"
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <map>
#include "crypto_utils.h"
#include "network.h"
#include "network_types.h"
#include "paxos.h"
#include "tcpserver.h"
#include "ui_dialog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->plainTextEdit->installEventFilter(this);

    QStringList list_of_friends = {"Dialog 1", "Dialog 2", "Dialog 3",
                                   "Dialog 4", "Dialog 5"};
    // ui->listWidget->setMinimumHeight(100);

    foreach (QString name, list_of_friends) {
        QListWidgetItem *item = new QListWidgetItem(name);
        ui->listWidget->addItem(item);
        item->setSizeHint(QSize(0, 50));
    }
    // connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this,
    //       SLOT(on_listWidget_itemClicked(QListWidgetItem *)));

    QPixmap pixmap("../untitled1/plus2.png");
    QIcon ButtonIcon(pixmap);

    ui->pushButton_2->setIcon(ButtonIcon);
    ui->pushButton_2->setIconSize(pixmap.rect().size());

    QPixmap pixmap2("../untitled1/dots.png");
    QIcon ButtonIcon2(pixmap2);

    ui->pushButton->setIcon(ButtonIcon2);
    ui->pushButton->setIconSize(pixmap2.rect().size());
}

MainWindow::~MainWindow() {
    delete ui;
}
QListWidget *MainWindow::get_list() {
    return ui->listWidget;
}
void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item) {
    ui->label_2->setText(
        QString(id_to_history[item->text().toStdString()].c_str()));
    std::cout << item->text().toStdString() << " "
              << id_to_history[item->text().toStdString()] << std::endl;
    // ui->label_2->setText(QString("hehe"));
    // item->setSizeHint(QSize(0, 100));
}

bool MainWindow::eventFilter(QObject *object, QEvent *event) {
    if (object == ui->plainTextEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return) {
            QString tmp = ui->label_2->text();
            tmp.append(ui->plainTextEdit->toPlainText());
            ui->label_2->setText(tmp + "\n");
            ui->plainTextEdit->setPlainText("");

            //  auto my_keys = key_exchange::generate_key();
            /* messenger::network::send_dialog_msg(
                 &serv, std::string("andrew2"), std::string("hello andrew"),
                 serv.keys.first, serv.keys.second, serv.keys.first,
                 [](boost::system::error_code ec) {
                     std::cout << ec.message();
                     std::cout << "error" << std::endl;
                 });*/

            return true;
        } else {
            return QMainWindow::eventFilter(object, event);
        }
    } else {
        return QMainWindow::eventFilter(object, event);
    }
}

Dialog::Dialog() : ui(new Ui::Dialog) {
    ui->setupUi(this);
}

Dialog::Dialog(MainWindow *m_) : ui(new Ui::Dialog), m(m_) {
    ui->setupUi(this);
}

void MainWindow::on_pushButton_2_clicked() {
    Dialog *dialog = new Dialog(this);
    dialog->setWindowTitle("Добавить пользователя");
    dialog->show();
}

void MainWindow::addItem(QListWidgetItem *item) {
    ui->listWidget->addItem(item);
}

void MainWindow::add_key_dialog(std::string key, std::string dialog) {
    this->get_dialog_by_key[key] = dialog;
}

void MainWindow::add_key_name(std::string key, std::string name) {
    this->get_name_by_key[key] = name;
}

void Dialog::on_pushButton_clicked() {
    this->close();
    QListWidgetItem *item = new QListWidgetItem(ui->lineEdit->text());
    item->setSizeHint(QSize(0, 50));
    m->add_key_name(ui->lineEdit_2->text().toStdString(),
                    ui->lineEdit->text().toStdString());
    m->addItem(item);
    emit m->button_close_addition(ui->lineEdit->text().toStdString(),
                                  ui->lineEdit_2->text().toStdString(),
                                  ui->lineEdit_3->text().toStdString());
}

std::map<std::string, std::string> &MainWindow::get_history() {
    return id_to_history;
}
void MainWindow::on_pushButton_4_clicked() {
    QString tmp = ui->label_2->text();
    tmp.append(ui->plainTextEdit->toPlainText());
    ui->label_2->setText(tmp + "\n");
    // std::cout << "here";
    std::string id = ui->listWidget->currentItem()->text().toStdString();
    std::string txt = ui->plainTextEdit->toPlainText().toStdString();
    id_to_history[id] += txt + "\n";  // decoration, message can be lost
    ui->plainTextEdit->setPlainText("");
    emit button_send(id, txt);
}

void Dialog::on_pushButton_2_clicked() {
    QString fileName =
        QFileDialog::getOpenFileName(this, "Load key", QDir::homePath());
    QMessageBox::information(this, "..", fileName);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "title", "file not open");
    }
    QTextStream in(&file);
    QString text = in.readAll();
    ui->lineEdit_2->setText(text);
    // ui->label_2->setText(text);

    file.close();
}
