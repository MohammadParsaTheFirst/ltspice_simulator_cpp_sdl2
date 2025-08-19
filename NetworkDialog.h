#ifndef NETWORKDIALOG_H
#define NETWORKDIALOG_H

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class NetworkDialog : public QDialog {
    Q_OBJECT

public:
    explicit NetworkDialog(QWidget *parent = nullptr);

    bool isServerMode() const;
    QString getIpAddress() const;
    int getPort() const;

private:
    QRadioButton *serverRadioButton;
    QRadioButton *clientRadioButton;
    QLineEdit *ipAddressEdit;
    QLineEdit *portEdit;
    QDialogButtonBox *buttonBox;
};

#endif // NETWORKDIALOG_H