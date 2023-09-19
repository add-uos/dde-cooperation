#ifndef PROMPTWIDGET_H
#define PROMPTWIDGET_H

#include <QFrame>

class PromptWidget : public QFrame
{
    Q_OBJECT

public:
    PromptWidget(QWidget *parent = nullptr);
    ~PromptWidget();

public slots:
    void nextPage();
    void backPage();

private:
    void initUI();
};

#endif
