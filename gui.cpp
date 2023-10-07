#include <QtCore>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QString>
#include <QTimer>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gui.hpp"
MainFrame::MainFrame(QWidget *parent) : QWidget(parent){
    prefix = "";
    pref = 0;
    blocksize = 1;
    blocksize <<= 60;
    upperbound = 0;
    upperbound--;
    fd = mknod("/tmp/fifo_file", S_IFIFO|0640, 0);
    file = open("/tmp/fifo_file", O_RDONLY|O_NONBLOCK);
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
    recalculate();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readMemoryAccess()));
    timer->start(100);
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
    recalculate();
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
    recalculate();
}

void MainFrame::recalculate(){
    memset(values, 0, sizeof(values));
    auto it = intervals.lower_bound(pref);
    if(it != intervals.begin()){
        it--;
        if(it->second < pref)
            it++;
    }
    int ind{};
    unsigned long long blockleft = pref;
    unsigned long long blockright = pref-1+blocksize;
    for(; it != intervals.end() && it->first <= upperbound; ++it){
        while(ind < 4096 && blockright < it->first){
            ind++;
            blockleft += blocksize;
            blockright += blocksize;
        }
        while(ind < 4096 && it->second > blockright){
            values[ind] += std::min(it->second, blockright) - std::max(it->first, blockleft) + 1;
            ind++;
            blockleft += blocksize;
            blockright += blocksize;
        }
        if(ind < 4096 && it->second >= blockleft){
            values[ind] += std::min(it->second, blockright) - std::max(it->first, blockleft) + 1;
        }
    }

    if(prefix.size() == 0){
        for(int i{}; i < 64; ++i){
            for(int j{}; j < 64; ++j){
                int place = (i/16)*4 + (j%64)/16;
                if(values[place] == 0){
                    grid->itemAtPosition(i, j)->widget()->setStyleSheet("background-color:rgb(0, 0, 0); border:none;");
                }else{
                    unsigned long long red = 255*(((long double)values[place])/blocksize);
                    unsigned long long green = 255-red;
                    grid->itemAtPosition(i, j)->widget()->setStyleSheet(QString("background-color:rgb(%1, %2, 0); border:none;").arg(red).arg(green));
                }
            }
        }
    }else{
        for(int i{}; i < 64; ++i){
            for(int j{}; j < 64; ++j){
                int place = i*64+j;
                if(values[place] == 0){
                    grid->itemAtPosition(i, j)->widget()->setStyleSheet("background-color:rgb(0, 0, 0); border:none;");
                }else{
                    unsigned long long red = 255*(((long double)values[place])/blocksize);
                    unsigned long long green = 255-red;
                    grid->itemAtPosition(i, j)->widget()->setStyleSheet(QString("background-color:rgb(%1, %2, 0); border:none;").arg(red).arg(green));
                }
            }
        }
    }
}

void other(int fd){
    close(fd);
}

void MainFrame::calcAndAdd(unsigned long long left, unsigned long long right, bool sub = false){
    if(left <= pref && right >= upperbound){
        for(int i{}; i < 4096; ++i){
            if(sub){
                values[i] = 0;
                grid->itemAtPosition(i/64, i%64)->widget()->setStyleSheet("background-color:rgb(0, 0, 0); border:none;");
            }else{
                values[i] = blocksize;
                grid->itemAtPosition(i/64, i%64)->widget()->setStyleSheet("background-color:rgb(255, 0, 0); border:none;");
            }
        }
    }else if(right < pref || left > upperbound){
        return;
    }else{
        unsigned long long leftind = (left-pref)/blocksize;
        unsigned long long rightind = (right-pref)/blocksize;
        for(unsigned long long i = leftind; i <= rightind; ++i){
            unsigned long long lbound = pref + (blocksize*i);
            unsigned long long rbound = lbound+blocksize-1;

            if(sub)
                values[i] -= std::min(right, rbound) - std::max(left, lbound)+1;
            else
                values[i] += std::min(right, rbound) - std::max(left, lbound)+1;
            unsigned long long red = 255*(((long double)values[i])/blocksize);
            unsigned long long green = 255-red;
            if(prefix.size() == 0){
                for(unsigned long long j = (i/4)*16; j < (i/4)*16+16; ++j){
                    for(unsigned long long k = (i%4)*16; k < (i%4)*16+16; ++k){
                        grid->itemAtPosition(j, k)->widget()->setStyleSheet(QString("background-color:rgb(%1, %2, 0); border:none;").arg(red).arg(green));
                    }
                }
            }else{
                grid->itemAtPosition(i/64, i%64)->widget()->setStyleSheet(QString("background-color:rgb(%1, %2, 0); border:none;").arg(red).arg(green));
            }

        }
    }
}

void MainFrame::readMemoryAccess(){
    for(int i{}; i < 100; ++i){
        int read_bytes = read(file, readbuf, sizeof(readbuf));
        if(read_bytes <= 0)
            break;
        if(readbuf[1] == 0){
            calcAndAdd(readbuf[0], intervals[readbuf[0]], true);
            intervals.erase(readbuf[0]);
        }else{
            calcAndAdd(readbuf[0], readbuf[0]+readbuf[1]-1);
            intervals[readbuf[0]] = readbuf[0]+readbuf[1]-1;
        }
    }
}

MainFrame::~MainFrame(){
    other(file);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainFrame window;
    window.setWindowTitle("Memory Usage Visualizer");
    window.show();

    return app.exec();
}
