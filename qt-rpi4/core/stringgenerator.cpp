#include "stringgenerator.h"

StringGenerator::StringGenerator(QObject *parent)
    : QObject{parent}
{

}

QString StringGenerator::generateWelcomeMessage()
{
    return "Welcome to Qt!";
}
