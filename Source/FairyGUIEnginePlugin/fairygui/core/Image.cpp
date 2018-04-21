#include "Image.h"
#include "NGraphics.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

static int gridTileIndice[] = { -1, 0, -1, 2, 4, 3, -1, 1, -1 };
static float gridX[4];
static float gridY[4];
static float gridTexX[4];
static float gridTexY[4];

Image::Image() :
    _scale9Grid(nullptr),
    _scaleByTile(false),
    _tileGridIndice(0),
    _flip(FlipType::NONE)
{
    _touchDisabled = true;
    _graphics = new NGraphics();
    _color.SetRGBA(0xFFFFFFFF);
}

Image::~Image()
{
    CC_SAFE_DELETE(_scale9Grid);
}

void Image::setTexture(NTexture* value)
{
    if (value == _graphics->getTexture())
        return;

    _graphics->setTexture(value);
    _requireUpdateMesh = true;
    if (_contentRect.GetSizeX() == 0)
        setNativeSize();
}

void Image::setNativeSize()
{
    float oldWidth = _contentRect.GetSizeX();
    float oldHeight = _contentRect.GetSizeY();
    if (_graphics->getTexture() != nullptr)
    {
        _contentRect.m_vMax.x = (float)_graphics->getTexture()->getWidth();
        _contentRect.m_vMax.y = (float)_graphics->getTexture()->getHeight();
    }
    else
    {
        _contentRect.m_vMax.x = 0;
        _contentRect.m_vMax.y = 0;
    }
    if (oldWidth != _contentRect.m_vMax.x || oldHeight != _contentRect.m_vMax.y)
        onSizeChanged(true, true);
}

void Image::setColor(const VColorRef & value)
{
    _color = value;
    _graphics->tint(value);
}

void Image::setFlip(FlipType value)
{
    if (_flip != value)
    {
        _flip = value;
        _requireUpdateMesh = true;
    }
}

const VRectanglef & Image::getScale9Grid() const
{
    if (_scale9Grid)
        return *_scale9Grid;
    else
        return EMPTY_RECT;
}

void Image::setScale9Grid(const VRectanglef& value)
{
    if (value.GetSizeX() > 0 && value.GetSizeY() > 0)
    {
        if (!_scale9Grid)
            _scale9Grid = new VRectanglef();
        *_scale9Grid = value;
    }
    else
    {
        CC_SAFE_DELETE(_scale9Grid);
    }
}

void Image::setScaleByTile(bool value)
{
    if (_scaleByTile != value)
    {
        _scaleByTile = value;
        _requireUpdateMesh = true;
    }
}

void Image::setTileGridIndice(int value)
{
    if (_tileGridIndice != value)
    {
        _tileGridIndice = value;
        _requireUpdateMesh = true;
    }
}

void Image::update(float dt)
{
    if (_requireUpdateMesh)
        rebuild();

    DisplayObject::update(dt);
}

void Image::rebuild()
{
    _requireUpdateMesh = false;
    _graphics->clearMesh();

    NTexture* texture = _graphics->getTexture();
    if (texture == nullptr)
        return;

    VRectanglef uvRect = texture->getUVRect();
    if (_flip != FlipType::NONE)
        ToolSet::flipRect(uvRect, _flip);

    /*if (_fillMethod != FillMethod.None)
    {
    graphics.Fill(_fillMethod, _fillAmount, _fillOrigin, _fillClockwise, _contentRect, uvRect);
    graphics.FillColors(_color);
    graphics.FillTriangles();
    if (_texture.rotated)
    NGraphics.RotateUV(graphics.uv, ref uvRect);
    graphics.UpdateMesh();
    }
    else */if (texture->getWidth() == _contentRect.GetSizeX() && texture->getHeight() == _contentRect.GetSizeY())
    {
        _graphics->addQuad(_contentRect, uvRect, _color);
    }
    else if (_scaleByTile)
    {
        tileFill(_contentRect, uvRect, (float)texture->getWidth(), (float)texture->getHeight());
    }
    else if (_scale9Grid != nullptr)
    {
        if (_flip != FlipType::NONE)
            ToolSet::flipInnerRect((float)texture->getWidth(), (float)texture->getHeight(), *_scale9Grid, _flip);

        generateGrids(*_scale9Grid, uvRect);

        if (_tileGridIndice == 0)
        {
            for (int i = 0; i < 9; i++)
            {
                int col = i % 3;
                int row = i / 3;

                _graphics->addQuad(VRectanglef(gridX[col], gridY[row], gridX[col + 1], gridY[row + 1]),
                    VRectanglef(gridTexX[col], gridTexY[row], gridTexX[col + 1], gridTexY[row + 1]),
                    _color);
            }
        }
        else
        {
            VRectanglef drawRect;
            VRectanglef texRect;
            int row, col;
            int part;

            for (int pi = 0; pi < 9; pi++)
            {
                col = pi % 3;
                row = pi / 3;
                part = gridTileIndice[pi];
                drawRect.Set(gridX[col], gridY[row], gridX[col + 1], gridY[row + 1]);
                texRect.Set(gridTexX[col], gridTexY[row], gridTexX[col + 1], gridTexY[row + 1]);

                if (part != -1 && (_tileGridIndice & (1 << part)) != 0)
                {
                    tileFill(drawRect, texRect,
                        (part == 0 || part == 1 || part == 4) ? _scale9Grid->GetSizeX() : drawRect.GetSizeX(),
                        (part == 2 || part == 3 || part == 4) ? _scale9Grid->GetSizeY() : drawRect.GetSizeY());
                }
                else
                {
                    _graphics->addQuad(drawRect, uvRect, _color);
                }
            }
        }
    }
    else
    {
        _graphics->addQuad(_contentRect, uvRect, _color);
    }

    if (texture->isRotated())
        _graphics->rotateUV(uvRect);
}

void Image::tileFill(const VRectanglef& destRect, const VRectanglef& uvRect, float sourceW, float sourceH)
{
    int hc = (int)hkvMath::ceil(destRect.GetSizeX() / sourceW);
    int vc = (int)hkvMath::ceil(destRect.GetSizeY() / sourceH);
    float tailWidth = destRect.GetSizeX() - (hc - 1) * sourceW;
    float tailHeight = destRect.GetSizeY() - (vc - 1) * sourceH;

    for (int i = 0; i < hc; i++)
    {
        for (int j = 0; j < vc; j++)
        {
            VRectanglef uvTmp = uvRect;
            if (i == hc - 1)
                uvTmp.m_vMax.x = VLerp<float>()(uvRect.m_vMin.x, uvRect.m_vMax.x, tailWidth / sourceW);
            if (j == vc - 1)
                uvTmp.m_vMax.y = VLerp<float>()(uvRect.m_vMin.y, uvRect.m_vMax.y, tailHeight / sourceH);
            float x = destRect.m_vMin.x + i * sourceW;
            float y = destRect.m_vMin.y + j * sourceH;
            _graphics->addQuad(VRectanglef(x, y, x + (i == (hc - 1) ? tailWidth : sourceW), y + (j == (vc - 1) ? tailHeight : sourceH)), uvTmp, _color);
        }
    }
}

void Image::generateGrids(const VRectanglef& gridRect, const VRectanglef& uvRect)
{
    NTexture* texture = _graphics->getTexture();
    float sx = uvRect.GetSizeX() / (float)texture->getWidth();
    float sy = uvRect.GetSizeY() / (float)texture->getHeight();
    gridTexX[0] = uvRect.m_vMin.x;
    gridTexX[1] = uvRect.m_vMin.x + gridRect.m_vMin.x * sx;
    gridTexX[2] = uvRect.m_vMin.x + gridRect.m_vMax.x * sx;
    gridTexX[3] = uvRect.m_vMax.x;
    gridTexY[0] = uvRect.m_vMin.y;
    gridTexY[1] = uvRect.m_vMin.y + gridRect.m_vMin.y * sy;
    gridTexY[2] = uvRect.m_vMin.y + gridRect.m_vMax.y * sy;
    gridTexY[3] = uvRect.m_vMax.y;

    gridX[0] = 0;
    gridY[0] = 0;
    if (_contentRect.GetSizeX() >= (texture->getWidth() - gridRect.GetSizeX()))
    {
        gridX[1] = gridRect.m_vMin.x;
        gridX[2] = _contentRect.GetSizeX() - (texture->getWidth() - gridRect.m_vMax.x);
        gridX[3] = _contentRect.GetSizeX();
    }
    else
    {
        float tmp = gridRect.m_vMin.x / (texture->getWidth() - gridRect.m_vMax.x);
        tmp = _contentRect.GetSizeX() * tmp / (1 + tmp);
        gridX[1] = tmp;
        gridX[2] = tmp;
        gridX[3] = _contentRect.GetSizeX();
    }

    if (_contentRect.GetSizeY() >= (texture->getHeight() - gridRect.GetSizeY()))
    {
        gridY[1] = gridRect.m_vMin.y;
        gridY[2] = _contentRect.GetSizeY() - (texture->getHeight() - gridRect.m_vMax.y);
        gridY[3] = _contentRect.GetSizeY();
    }
    else
    {
        float tmp = gridRect.m_vMin.y / (texture->getHeight() - gridRect.m_vMax.y);
        tmp = _contentRect.GetSizeY() * tmp / (1 + tmp);
        gridY[1] = tmp;
        gridY[2] = tmp;
        gridY[3] = _contentRect.GetSizeY();
    }
}

NS_FGUI_END