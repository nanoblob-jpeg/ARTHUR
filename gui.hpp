#pragma once
#include <QtCore>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QSignalMapper>
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <fcntl.h>


class MainFrame : public QWidget {
    Q_OBJECT
public:
    MainFrame(QWidget *parent = nullptr);
    ~MainFrame();

private slots:
    void clicked(int);
    void back();
    void readMemoryAccess();

private:
    void calcAndAdd(unsigned long long left, unsigned long long right, bool sub);
    QSignalMapper *mapper;
    QVBoxLayout *vbox;
    QGridLayout *grid;
    unsigned long long values[4096];
    std::string prefix;
    unsigned long long pref;
    unsigned long long upperbound;
    unsigned long long blocksize;
    int fd;
    int file;
    unsigned long long readbuf[2];
    std::map<unsigned long long, unsigned long long> intervals;
    std::vector<std::string> hex{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};

    void recalculate();
};

