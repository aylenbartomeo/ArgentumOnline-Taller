#include "Launcher.h"

#include <string>
#include <utility>

#include "ui_launcher.h"

Launcher::Launcher(QWidget* parent):
        QWidget(parent), ui(new Ui::Form), client(nullptr), authenticated(false) {
    ui->setupUi(this);
    this->setWindowTitle("Argentum Online - Conexión");
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->connectBtn, &QPushButton::clicked, this, &Launcher::handleConnectionStage);
    connect(ui->loginBtn, &QPushButton::clicked, this,
            [this]() { handleAuthenticationStage("login"); });
    connect(ui->registerBtn, &QPushButton::clicked, this,
            [this]() { handleAuthenticationStage("register"); });
}

Launcher::~Launcher() { delete ui; }

// ETAPA 1: Conexión al Host/Puerto
void Launcher::handleConnectionStage() {
    std::string host = ui->ipInput->text().toStdString();
    std::string port = ui->portInput->text().toStdString();

    if (host.empty() || port.empty()) {
        ui->connectionStatusLabel->setText("Campos incompletos.");
        return;
    }

    try {
        this->client = std::make_unique<Client>(host.c_str(), port.c_str());

        ui->connectionStatusLabel->setText("");
        this->setWindowTitle("Argentum Online - Autenticación");
        ui->stackedWidget->setCurrentIndex(1);

    } catch (const std::exception& e) {
        ui->connectionStatusLabel->setText("Error de red: No se pudo conectar al host.");
        this->client.reset();  // Destruimos el intento si falló
    }
}

// ETAPA 2: Autenticación (Login / Registro)
void Launcher::handleAuthenticationStage(const std::string& action) {
    std::string user = ui->usernameInput->text().toStdString();
    std::string pass = ui->passwordInput->text().toStdString();

    if (user.empty() || pass.empty()) {
        ui->authStatusLabel->setText("Usuario o contraseña vacíos.");
        return;
    }

    std::string mensajeErrorDelServer = "";

    if (this->client->authenticate(action, user, pass, mensajeErrorDelServer)) {
        this->authenticated = true;
        this->close();
    } else {
        ui->authStatusLabel->setText(QString::fromStdString(mensajeErrorDelServer));
    }
}

bool Launcher::isAuthenticated() const { return this->authenticated; }

std::unique_ptr<Client> Launcher::releaseClient() { return std::move(this->client); }
