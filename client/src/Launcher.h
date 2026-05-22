#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QWidget>
#include <memory>
#include <string>

#include "Client.h"

namespace Ui {
class Form;
}

class Launcher: public QWidget {
    Q_OBJECT

public:
    explicit Launcher(QWidget* parent = nullptr);
    ~Launcher();

    bool isAuthenticated() const;

    std::unique_ptr<Client> releaseClient();

    // cppcheck-suppress unknownMacro
private slots:
    void handleConnectionStage();
    void handleAuthenticationStage(const std::string& action);

private:
    Ui::Form* ui;
    std::unique_ptr<Client> client;
    bool authenticated;
};

#endif
