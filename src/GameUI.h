#ifndef GAME_UI_H
#define GAME_UI_H

#include <string>

class GameUI {
public:
    virtual ~GameUI() = default;

    virtual void clear() = 0;
    virtual void hr(const char* pattern = "-", const char* colorCode = nullptr) = 0;
    virtual void typewriteWords(const std::string& text, int spaceMs = 28, int newlineMs = 110) = 0;
    virtual void typewriteJitter(const std::string& text, int baseMs = 22, int jitterMs = 12) = 0;
    virtual void print(const std::string& text) = 0;
    virtual void printLine(const std::string& text) = 0;
    virtual void flush() = 0;
    virtual bool readToken(std::string& out) = 0;
    virtual bool readInt(int& out) = 0;
    virtual bool readLine(std::string& out) = 0;
    virtual void discardLine() = 0;
};

class ConsoleGameUI : public GameUI {
public:
    void clear() override;
    void hr(const char* pattern = "-", const char* colorCode = nullptr) override;
    void typewriteWords(const std::string& text, int spaceMs = 28, int newlineMs = 110) override;
    void typewriteJitter(const std::string& text, int baseMs = 22, int jitterMs = 12) override;
    void print(const std::string& text) override;
    void printLine(const std::string& text) override;
    void flush() override;
    bool readToken(std::string& out) override;
    bool readInt(int& out) override;
    bool readLine(std::string& out) override;
    void discardLine() override;
};

GameUI& activeGameUI();
void setActiveGameUI(GameUI* ui);

#endif // GAME_UI_H
