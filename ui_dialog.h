/********************************************************************************
** Form generated from reading UI file 'dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.2.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *lineEdit_2;
    QPushButton *pushButton_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QLineEdit *lineEdit_3;
    QPushButton *pushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(493, 344);
        verticalLayoutWidget = new QWidget(Dialog);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(20, 10, 451, 306));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font;
        font.setFamilies({QString::fromUtf8("SF UI Display")});
        font.setPointSize(12);
        font.setBold(false);
        label->setFont(font);

        verticalLayout->addWidget(label, 0, Qt::AlignBottom);

        lineEdit = new QLineEdit(verticalLayoutWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setFont(font);

        verticalLayout->addWidget(lineEdit);

        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        verticalLayout->addWidget(label_2, 0, Qt::AlignBottom);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(-1, 0, -1, 20);
        lineEdit_2 = new QLineEdit(verticalLayoutWidget);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        QFont font1;
        font1.setFamilies({QString::fromUtf8("SF UI Display")});
        font1.setPointSize(12);
        lineEdit_2->setFont(font1);

        horizontalLayout_2->addWidget(lineEdit_2);

        pushButton_2 = new QPushButton(verticalLayoutWidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setFont(font1);

        horizontalLayout_2->addWidget(pushButton_2);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 5, -1, 0);
        label_3 = new QLabel(verticalLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setFont(font);

        horizontalLayout->addWidget(label_3);

        radioButton = new QRadioButton(verticalLayoutWidget);
        radioButton->setObjectName(QString::fromUtf8("radioButton"));
        radioButton->setFont(font);

        horizontalLayout->addWidget(radioButton);

        radioButton_2 = new QRadioButton(verticalLayoutWidget);
        radioButton_2->setObjectName(QString::fromUtf8("radioButton_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(radioButton_2->sizePolicy().hasHeightForWidth());
        radioButton_2->setSizePolicy(sizePolicy1);
        radioButton_2->setFont(font);

        horizontalLayout->addWidget(radioButton_2);


        verticalLayout->addLayout(horizontalLayout);

        lineEdit_3 = new QLineEdit(verticalLayoutWidget);
        lineEdit_3->setObjectName(QString::fromUtf8("lineEdit_3"));
        lineEdit_3->setFont(font);

        verticalLayout->addWidget(lineEdit_3);

        pushButton = new QPushButton(verticalLayoutWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setFont(font1);

        verticalLayout->addWidget(pushButton, 0, Qt::AlignBottom);


        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QCoreApplication::translate("Dialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("Dialog", "\320\222\320\262\320\265\320\264\320\270\321\202\320\265 \320\270\320\274\321\217", nullptr));
        label_2->setText(QCoreApplication::translate("Dialog", "\320\237\321\203\320\261\320\273\320\270\321\207\320\275\321\213\320\271 \320\272\320\273\321\216\321\207 \320\277\320\276\320\273\321\214\320\267\320\276\320\262\320\260\321\202\320\265\320\273\321\217", nullptr));
        pushButton_2->setText(QCoreApplication::translate("Dialog", "\320\222\321\213\320\261\321\200\320\260\321\202\321\214 \320\272\320\273\321\216\321\207 \320\270\320\267 \321\204\320\260\320\271\320\273\320\260", nullptr));
        label_3->setText(QCoreApplication::translate("Dialog", "IP / \320\260\320\264\321\200\320\265\321\201 \320\262 I2P", nullptr));
        radioButton->setText(QCoreApplication::translate("Dialog", "I2P", nullptr));
        radioButton_2->setText(QCoreApplication::translate("Dialog", "TCP/IP", nullptr));
        pushButton->setText(QCoreApplication::translate("Dialog", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214", nullptr));
    } // retranslateUi

};

namespace Ui {
class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_H
