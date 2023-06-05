/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.2.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QListWidget *listWidget;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QLabel *label;
    QPushButton *pushButton_2;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QPlainTextEdit *plainTextEdit;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_2;
    QPushButton *pushButton_4;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(986, 598);
        MainWindow->setStyleSheet(QString::fromUtf8(""));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayoutWidget = new QWidget(centralwidget);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(20, 60, 291, 481));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        listWidget = new QListWidget(gridLayoutWidget);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        QFont font;
        font.setFamilies({QString::fromUtf8("SF UI Display")});
        font.setPointSize(12);
        font.setBold(false);
        font.setItalic(false);
        listWidget->setFont(font);
        listWidget->setStyleSheet(QString::fromUtf8(""));

        gridLayout->addWidget(listWidget, 0, 0, 1, 1);

        horizontalLayoutWidget = new QWidget(centralwidget);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(20, 10, 291, 35));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        pushButton = new QPushButton(horizontalLayoutWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy);
        pushButton->setAutoFillBackground(false);
        pushButton->setStyleSheet(QString::fromUtf8("QPushButton\n"
            "{\n"
            "border: none\n"
            "}\n"
            "QPushButton:hover\n"
            "{\n"
            "border: 1px solid #bebebe;\n"
            "}"));
        pushButton->setCheckable(false);

        horizontalLayout->addWidget(pushButton);

        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font);

        horizontalLayout->addWidget(label, 0, Qt::AlignHCenter);

        pushButton_2 = new QPushButton(horizontalLayoutWidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButton_2->sizePolicy().hasHeightForWidth());
        pushButton_2->setSizePolicy(sizePolicy1);
        pushButton_2->setStyleSheet(QString::fromUtf8("QPushButton\n"
            "{\n"
            "border: none\n"
            "}\n"
            "QPushButton:hover\n"
            "{\n"
            "border: 1px solid #bebebe;\n"
            "}\n"
            ""));

        horizontalLayout->addWidget(pushButton_2);

        verticalLayoutWidget = new QWidget(centralwidget);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(330, 510, 601, 31));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        plainTextEdit = new QPlainTextEdit(verticalLayoutWidget);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Ignored);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(plainTextEdit->sizePolicy().hasHeightForWidth());
        plainTextEdit->setSizePolicy(sizePolicy2);
        plainTextEdit->setFont(font);
        plainTextEdit->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);\n"
            "border: 0.5px solid grey;\n"
            "border-top-color: rgba(255, 255, 255, 0);\n"
            "border-right-color: rgba(255, 255, 255, 0);\n"
            "border-left-color: rgba(255, 255, 255, 0);"));
        plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        verticalLayout->addWidget(plainTextEdit);

        verticalLayoutWidget_2 = new QWidget(centralwidget);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(329, 9, 601, 501));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(verticalLayoutWidget_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        verticalLayout_2->addWidget(label_2);

        pushButton_4 = new QPushButton(centralwidget);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setGeometry(QRect(940, 520, 20, 21));
        QFont font1;
        font1.setPointSize(14);
        pushButton_4->setFont(font1);
        pushButton_4->setStyleSheet(QString::fromUtf8("QPushButton\n"
            "{\n"
            "border: none\n"
            "}\n"
            "QPushButton:hover\n"
            "{\n"
            "border: 1px solid #bebebe;\n"
            "}\n"
            ""));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 986, 29));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        pushButton->setDefault(false);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton->setText(QString());
        label->setText(QCoreApplication::translate("MainWindow", "FFLC messenger", nullptr));
        pushButton_2->setText(QString());
        label_2->setText(QCoreApplication::translate("MainWindow", "\320\241\320\276\320\276\320\261\321\211\320\265\320\275\320\270\320\265 1         ", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "\342\207\250", nullptr));
    } // retranslateUi

};

namespace Ui {
class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
