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
#include <QTabWidget>
#include <QComboBox>
#include "Circuit.h"

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
    explicit NodeLibraryDialog(Circuit* circuit, QWidget *parent = Q_NULLPTR);

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

class ConfigureAnalysisDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConfigureAnalysisDialog(QWidget *parent = Q_NULLPTR);
    int getSelectedAnalysisType() const;
    QString getTransientTstop() const;
    QString getTransientTstart() const;
    QString getTransientTstep() const;
    QString getTransientParameter() const;
    QString getACOmegaStart() const;
    QString getACOmegaStop() const;
    QString getACNPoints() const;
    QString getACParameter() const;
    QString getPhaseBaseFrequency() const;
    QString getPhaseStart() const;
    QString getPhaseStop() const;
    QString getPhaseNPoints() const;

private:
    QTabWidget* tabWidget;

    // Storing the transient analysis type variables
    QLineEdit* tStopEdit;
    QLineEdit* tStartEdit;
    QLineEdit* tStepEdit;
    QLineEdit* transientParameterEdit;

    // AC Sweep
    QComboBox* typeOfSweepComboBox;
    QLineEdit* ACOmegaStart;
    QLineEdit* ACOmegaStop;
    QLineEdit* ACNPoint;
    QLineEdit* ACSweepParameterEdit;

    // Phase Sweep - menu should be in the project but its analyze not
    QLineEdit* phaseBaseFrequency;
    QLineEdit* phaseStart;
    QLineEdit* phaseStop;
    QLineEdit* phaseNPoints;
    QLineEdit* phaseParameterEdit;

    QDialogButtonBox *buttonBox;
};

class SubcircuitLibarary : public QDialog {
    Q_OBJECT
public:
    explicit SubcircuitLibarary(Circuit* circuit, QWidget *parent = Q_NULLPTR);

signals:
        void componentSelected(const QString& componentType);
private slots:
        void doubleClickedOnItem(QListWidgetItem* item);

private:
    QListWidget* listWidget;
};
#endif //VALUEDIALOG_H