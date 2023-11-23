#ifndef STRINGGENERATOR_H
#define STRINGGENERATOR_H

#include <QObject>

class StringGenerator : public QObject
{
    Q_OBJECT
public:
    explicit StringGenerator(QObject *parent = nullptr);

    QString generateWelcomeMessage();

signals:

};

#endif // STRINGGENERATOR_H
