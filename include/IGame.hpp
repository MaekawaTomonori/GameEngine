#ifndef IGame_HPP_
#define IGame_HPP_
#include <string>
#include <cstdint>

class IGame {
protected:
	// Protected inner Config
	class Config {
		uint16_t fps_ = 60;
		std::string title_;
	public:
	    Config() = default;
		Config(std::string _title, uint16_t _fps = 60);
	};
private:
	Config config_;
public:
	IGame();
	virtual ~IGame() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Shutdown() = 0;

protected:
	Config& SetConfig(const Config& _config);
	Config& GetConfig();
}; // class IGame

#endif // IGame_HPP_
