#ifndef UNITINDEXMOVE_H
#define UNITINDEXMOVE_H
#include "Reversibles.h"
#include "UndoTypes.h"

class UnitIndexMove : public ReversibleAction
{
    public:
        static std::shared_ptr<UnitIndexMove> Make(u16 oldIndex, u16 newIndex);
        virtual void Reverse(void *guiMap);
        virtual int32_t GetType();

    protected:
        UnitIndexMove(u16 oldIndex, u16 newIndex);

    private:
        u16 oldIndex;
        u16 newIndex;
};

class UnitIndexMoveBoundary : public ReversibleAction
{
    public:
        static std::shared_ptr<UnitIndexMoveBoundary> Make();
        virtual void Reverse(void *guiMap);
        virtual int32_t GetType();

    protected:
        UnitIndexMoveBoundary();
};

#endif