#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	CellOrigin = { -Width / 2, -Height / 2};

	// TODO create the cells
	for (int row = 0; row < NrOfRows; ++row)
	{
		for (int col = 0; col < NrOfCols; ++col)
		{
			float Left = CellOrigin.X + col * CellWidth;
			float Bottom = CellOrigin.Y + row * CellHeight;
			Cells.emplace_back(Left, Bottom, CellWidth, CellHeight);
		}
	}

}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	int Index = PositionToIndex(Agent.GetPosition());
	Cells[Index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	int OldIndex = PositionToIndex(OldPos);
	int NewIndex = PositionToIndex(Agent.GetPosition());

	if (OldIndex != NewIndex)
	{
		Cells[OldIndex].Agents.remove(&Agent);
		Cells[NewIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius, bool UsePartitioning)
{
	NrOfNeighbors = 0;

	FRect QueryRect(
		{ Agent.GetPosition().X - QueryRadius, Agent.GetPosition().Y - QueryRadius },
		{ QueryRadius * 2.f, QueryRadius * 2.f }
	);

	for (Cell& c : Cells)
	{
		if (NrOfNeighbors >= Neighbors.Num())
			return;

		if (!DoRectsOverlap(QueryRect, c.BoundingBox) && UsePartitioning)
			continue;

		for (ASteeringAgent* pNeighbor : c.Agents)
		{
			if (NrOfNeighbors >= Neighbors.Num())
				return;
			if (pNeighbor == &Agent)
				continue;

			float Distance = FVector2D::Distance(Agent.GetPosition(), pNeighbor->GetPosition());
			if (Distance < QueryRadius)
			{
				Neighbors[NrOfNeighbors] = pNeighbor;
				++NrOfNeighbors;
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells(bool UsePartitioning) const
{
	if (!UsePartitioning)
		return;
	for (const Cell& c : Cells)
	{
		// Draw the grid
		std::vector<FVector2D> Points = c.GetRectPoints();

		for (int i = 0; i < Points.size(); ++i)
		{
			FVector Start = FVector(Points[i], 0.f);
			FVector End = FVector(Points[(i + 1) % Points.size()], 0.f);
			DrawDebugLine(pWorld, Start, End, FColor::White, false, -1.f, 0, 5.f);
		}

		// Show agent count in each cell
		DrawDebugString(pWorld, FVector(Points[3], 0.f), FString::FromInt(c.Agents.size()), nullptr, FColor::White, 0.1f);
	}

	// Draw the origin
	DrawDebugCircle(pWorld, FVector(CellOrigin, 0.f), 5.f, 12, FColor::Red, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	// TODO Calculate the index of the cell based on the position
	int Col = FMath::FloorToInt((Pos.X - CellOrigin.X) / CellWidth);
	int Row = FMath::FloorToInt((Pos.Y - CellOrigin.Y) / CellHeight);

	Col = FMath::Clamp(Col, 0, NrOfCols - 1);
	Row = FMath::Clamp(Row, 0, NrOfRows - 1);

	return Row * NrOfCols + Col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}