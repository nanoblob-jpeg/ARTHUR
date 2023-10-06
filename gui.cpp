#include <QtCore>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QString>
#include <iostream>
#include "gui.hpp"
MainFrame::MainFrame(QWidget *parent) : QWidget(parent){
    prefix = "";
    pref = 0;
    blocksize = 1;
    blocksize <<= 60;
    vbox = new QVBoxLayout(this);

    auto *memoryPrefixLabel = new QLabel("Prefix:", this);
    memoryPrefixLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mapper = new QSignalMapper(this);
    QObject::connect(mapper, SIGNAL(mapped(int)), this, SLOT(clicked(int)));
    grid = new QGridLayout();
    grid->setSpacing(0);
    for(int i{}; i < 64; ++i){
        for(int j{}; j < 64; ++j){
            auto *btn = new QPushButton("", this);
            btn->resize(4, 4);
            btn->clearMask();
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            btn->setStyleSheet("background-color: black; border: none;");
            btn->setMaximumSize(12, 12);
            QObject::connect(btn, SIGNAL(clicked()), mapper, SLOT(map()));
            mapper->setMapping(btn, i*64+j);
            grid->addWidget(btn, i, j);
        }
    }

    auto *goBack = new QPushButton("Back", this);
    goBack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(goBack, &QPushButton::clicked, this, &MainFrame::back);
    
    vbox->setSpacing(1);
    vbox->addWidget(memoryPrefixLabel);
    vbox->addLayout(grid);
    vbox->addWidget(goBack);

    setLayout(vbox);
}

void MainFrame::clicked(int index){
    if(prefix.size() >= 13)
        return;
    if(prefix.size() == 0){
        int i = (index/64)/16;
        int j = (index%64)/16;
        unsigned long long adding = (i<<2)|j;
        prefix += hex[adding];
        ((QLabel*)vbox->itemAt(0)->widget())->setText(QString::fromStdString("Prefix: " + prefix));
        pref = (adding<<60);
        blocksize >>= 12;
        upperbound = pref+blocksize*4096-1;
    }else{
        int i = index/64;
        int j = index%64;
        unsigned long long adding = (i<<6)|j;
        int numshift = 64-prefix.size()*4-12;
        prefix += hex[adding>>8] + hex[(adding>>4)&15] + hex[adding&15];
        ((QLabel*)vbox->itemAt(0)->widget())->setText(QString::fromStdString("Prefix: " + prefix));
        pref += (adding<<numshift);
        blocksize >>= 12;
        upperbound = pref+blocksize*4096-1;
    }
    //recalculate();
    std::cerr << prefix << ' ' << pref << ' ' << blocksize << ' ' << upperbound << '\n';
}

void MainFrame::back(){
    if(prefix.size() == 0)
        return;
    if(prefix.size() == 1){
        prefix = "";
        ((QLabel*)vbox->itemAt(0)->widget())->setText(QString::fromStdString("Prefix: " + prefix));
        pref = 0;
        blocksize = 1;
        blocksize <<= 60;
        upperbound = 0;
        upperbound--;
    }else{
        unsigned long long mask = 1;
        mask <<= (64-prefix.size()*4) + 12;
        mask--;
        mask = ~mask;
        pref &= mask;
        for(int i{}; i < 3; ++i)
            prefix.pop_back();
        ((QLabel*)vbox->itemAt(0)->widget())->setText(QString::fromStdString("Prefix: " + prefix));
        blocksize <<= 12;
        upperbound = pref+blocksize*4096-1;
    }
    //recalculate();
    std::cerr << prefix << ' ' << pref << ' ' << blocksize << ' ' << upperbound << '\n';
}

void MainFrame::recalculate(){
    auto it = intervals.lower_bound(pref);
    if(it != intervals.begin()){
        it--;
        if(it->second < pref)
            it++;
    }
    for(; it != intervals.end() && it->first <= pref-1+blocksize*4096; ++it){
        
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainFrame window;
    window.setWindowTitle("Memory Usage Visualizer");
    window.show();

    return app.exec();
}
