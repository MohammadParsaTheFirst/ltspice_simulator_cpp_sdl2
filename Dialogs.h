#ifndef VALUEDIALOG_H
#define VALUEDIALOG_H


#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QListWidget>

class ValueDialog : public QDialog {
    Q_OBJECT
public:
    explicit ValueDialog(QWidget *parent = Q_NULLPTR);

    QString getValue() const;

private:
    QLineEdit *valueEdit;
    QDialogButtonBox *buttonBox;
};

class SourceValueDialog : public QDialog {
    Q_OBJECT
public:
    explicit SourceValueDialog(QWidget *parent = Q_NULLPTR);

    bool isSinusoidal() const;
    QString getDCValue() const;
    QString getSinOffset() const;
    QString getSinAmplitude() const;
    QString getSinFrequency() const;

    private slots:
        void showSinOrNot(bool isSin);

private:
    QRadioButton* dcForm;
    QRadioButton* sinForm;

    QGroupBox* dcGroupBox;
    QLineEdit* dcInput;

    QGroupBox* sinGroupBox;
    QLineEdit* sinOffset;
    QLineEdit* sinAmplitude;
    QLineEdit* sinFrequency;

    QDialogButtonBox *buttonBox;
};

class NodeLibraryDialog : public QDialog {
    Q_OBJECT
public:
    explicit NodeLibraryDialog(QWidget *parent = Q_NULLPTR);

    signals:
        void componentSelected(const QString& componentType);

    private slots:
        void doubleClickedOnItem(QListWidgetItem* item);

private:
    QListWidget* listWidget;
};

class LabelDialog : public QDialog {
    Q_OBJECT
public:
    explicit LabelDialog(QWidget *parent = Q_NULLPTR);
    QString getLabel() const;

private:
    QLineEdit* labelLineEdit;
    QDialogButtonBox *labelButtonBox;
};

#endif //VALUEDIALOG_H