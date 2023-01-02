#include "../../../../stdafx.h"
#include "EGraphRenderer.h"
#include "IBaseInterface.h"

namespace Elite
{
	void GraphRenderer::RenderCircleNode(IBaseInterface* pInterface, Vector2 pos, std::string text /*= ""*/, float radius /*= 3.0f*/, Elite::Color col /*= DEFAULT_NODE_COLOR*/, float depth /*= 0.0f*/) const
	{
		pInterface->Draw_SolidCircle(pos, radius, {1,0}, {col.r, col.g, col.b});
	}

	void GraphRenderer::RenderRectNode(IBaseInterface* pInterface, Vector2 pos, std::string text /*= ""*/, float width /* = 3.0f*/, Elite::Color col /*= DEFAULT_NODE_COLOR*/, float depth /*= 0.0f*/) const
	{
		const Vector2 verts[4]
		{
			Vector2(pos.x - width / 2.0f, pos.y - width / 2.0f),
			Vector2(pos.x - width / 2.0f, pos.y + width / 2.0f),
			Vector2(pos.x + width / 2.0f, pos.y + width / 2.0f),
			Vector2(pos.x + width / 2.0f, pos.y - width / 2.0f)
		};

		pInterface->Draw_SolidPolygon(&verts[0], 4, { col.r, col.g, col.b }, depth);
	}

	void GraphRenderer::RenderConnection(IBaseInterface* pInterface, GraphConnection* con, Elite::Vector2 toPos, Elite::Vector2 fromPos, std::string text, Elite::Color col, float depth/*= 0.0f*/) const
	{
		auto center = toPos + (fromPos - toPos) / 2;

		pInterface->Draw_Segment(toPos, fromPos, { col.r, col.g, col.b }, depth);
	}
}