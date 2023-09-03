#include <iostream>
#include <fstream>

class LogMessage {

public:
    
    enum class Type {

        warning,
        error,
        fatal_error,
        unknown_message,
    };

    virtual ~LogMessage() = default;

    virtual Type type() const = 0;

    virtual const std::string message() const = 0;
};

class WarningMessage : public LogMessage {

public:

    Type type() const override {

        return Type::warning;
    }

    const std::string message() const override {

        return "The file is open";
    }
};

class ErrorsMessage : public LogMessage {

public:

    Type type() const override {

        return Type::error;
    }

    const std::string message() const override {

        return "End of file";
    }
};

class FatalErrorsMessage : public LogMessage {

public:

    Type type() const override {

        return Type::fatal_error;
    }

    const std::string message() const override {

        return "File opening error";
    }
};

class UnknownMessage : public LogMessage {

public:

    Type type() const override {

        return Type::unknown_message;
    }

    const std::string message() const override {

        return nullptr;
    }
};



class ErrorHandler {

public:

    explicit ErrorHandler(std::unique_ptr<ErrorHandler> _next) : next(std::move(_next)) {}

    virtual ~ErrorHandler() = default;

    void receiveError(const LogMessage& msg, std::ostream& console, std::ostream& file);

private:

    std::unique_ptr<ErrorHandler> next;

    virtual bool handleError(const LogMessage& msg, std::ostream& console, std::ostream& file) = 0;
};

class Warning : public ErrorHandler {

public:

    using ErrorHandler::ErrorHandler;

private:

    bool handleError(const LogMessage& msg, std::ostream& console, std::ostream& file) override;
};

class Errors : public ErrorHandler {

public:

    using ErrorHandler::ErrorHandler;

private:

    bool handleError(const LogMessage& msg, std::ostream& console, std::ostream& file) override;
};

class FatalErrors : public ErrorHandler {

public:

    using ErrorHandler::ErrorHandler;

private:

    bool handleError(const LogMessage& msg, std::ostream& console, std::ostream& file) override;
};



void ErrorHandler::receiveError(const LogMessage& msg, std::ostream& console, std::ostream& file) {

    if (handleError(msg, console, file)) {
    }
    else if (next) {

        next->receiveError(msg, console, file);
    }
    else {

        throw std::runtime_error("Error: The message was not processed!");
    }
}

bool Warning::handleError(const LogMessage& msg, std::ostream& console, std::ostream& file) {

    if (msg.type() != LogMessage::Type::warning) {

        return false;
    }

    console << "Warning: " << msg.message() << std::endl;

    return true;
}

bool Errors::handleError(const LogMessage& msg, std::ostream& console, std::ostream& file) {

    if (msg.type() != LogMessage::Type::error) {

        return false;
    }

    file << msg.message() << std::endl;

    return true;
}

bool FatalErrors::handleError(const LogMessage& msg, std::ostream& console, std::ostream& file) {

    if (msg.type() != LogMessage::Type::fatal_error) {

        return false;
    }

    std::string err("Fatal error: " + msg.message());
    throw std::runtime_error(err);

    return true;
}

int main() {

    std::fstream file("out.txt", std::ios::app);

    auto news_error = std::make_unique<Warning>(nullptr);
    auto warning = std::make_unique<Warning>(std::move(news_error));
    auto errors = std::make_unique<Errors>(std::move(warning));
    auto fatal_error = std::make_unique<FatalErrors>(std::move(errors));

    try {

        fatal_error->receiveError(WarningMessage(), std::cout, file);
        fatal_error->receiveError(ErrorsMessage(), std::cout, file);
        fatal_error->receiveError(UnknownMessage(), std::cout, file);
    }
    catch (const std::exception& err) {

        std::cout << err.what() << std::endl;
    }
}