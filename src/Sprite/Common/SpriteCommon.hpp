#ifndef SpriteCommon_HPP_
#define SpriteCommon_HPP_
#include "src/Common/Common.hpp"

class SpriteCommon : public Common{
public:
	void Initialize(const GESTD::WeakPtr<DirectXAdapter>& _adapter, const GESTD::WeakPtr<DebugUI>& _debugUi) override;
}; // class SpriteCommon

#endif // SpriteCommon_HPP_
