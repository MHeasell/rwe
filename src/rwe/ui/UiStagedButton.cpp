#include "UiStagedButton.h"

namespace rwe
{
    void UiStagedButton::render(GraphicsContext& graphics) const
    {
        auto spriteCount = spriteSeries->sprites.size();

        assert(spriteCount >= 3);
        auto pressedSprite = spriteCount - 2;

        assert(currentStage < spriteCount);
        auto stageSprite = currentStage;

        auto spriteIndex = pressed ? pressedSprite : stageSprite;
        const Sprite& sprite = spriteSeries->sprites[spriteIndex];

        graphics.drawTextureRegion(
                posX,
                posY,
                sprite.bounds.width(),
                sprite.bounds.height(),
                sprite.texture,
                sprite.textureRegion.left(),
                sprite.textureRegion.top(),
                sprite.textureRegion.width(),
                sprite.textureRegion.height()
        );

        float textX = posX + (sizeX / 2.0f);
        float textY = posY + (sizeY / 2.0f);
        graphics.drawTextCentered(textX, pressed ? textY + 1.0f : textY, labels[currentStage], *labelFont);
    }

    UiStagedButton::UiStagedButton(
        int posX,
        int posY,
        unsigned int sizeX,
        unsigned int sizeY,
        std::shared_ptr<SpriteSeries> _spriteSeries,
        std::vector<std::string> _labels,
        std::shared_ptr<SpriteSeries> _labelFont)
    : UiComponent(posX, posY, sizeX, sizeY),
      spriteSeries(std::move(_spriteSeries)),
      labels(std::move(_labels)),
      labelFont(std::move(_labelFont))
    {
        assert(labels.size() == spriteSeries->sprites.size() - 3);
    }
}