#ifndef TRANSIENTDIALOG_H
#define TRANSIENTDIALOG_H

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class TransientDialog : public QDialog {
    Q_OBJECT

public:
    explicit TransientDialog(QWidget *parent = nullptr);
    ~TransientDialog();

    double getStartTime() const;
    double getStopTime() const;
    double getStepTime() const;
    QString getParameter() const;

private:
    QLineEdit *startTimeEdit;
    QLineEdit *stopTimeEdit;
    QLineEdit *stepTimeEdit;
    QLineEdit *parameterEdit;
    QDialogButtonBox *buttonBox;
};

#endif // TRANSIENTDIALOG_H
