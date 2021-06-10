namespace bav
{
UndoManager::UndoManager (SerializableData& stateToManage)
    : state (stateToManage)
{
    saveState();
}

bool UndoManager::undo()
{
    if (hasUndo())
    {
        loadState (storedStates[--currentStep]);
        return true;
    }

    return false;
}

bool UndoManager::redo()
{
    if (hasRedo())
    {
        loadState (storedStates[++currentStep]);
        return true;
    }

    return false;
}

void UndoManager::undoToLastTransaction()
{
    loadState (storedStates[currentStep]);
}

bool UndoManager::hasUndo() const
{
    return currentStep - 1 >= 0 && ! storedStates.empty();
}

bool UndoManager::hasRedo() const
{
    return currentStep + 1 < storedStates.size();
}

void UndoManager::beginNewTransaction (const String& name, bool force)
{
    if (force || ! changing)
    {
        transactionName = name;
        changing        = true;
    }
}

void UndoManager::endTransaction()
{
    if (changing)
    {
        saveState();
        changing = false;
    }
}

void UndoManager::saveState()
{
    storedStates.emplace_back (state.serialize(), transactionName);
}

void UndoManager::clearUndoHistory()
{
    storedStates.clear();
    transactionName.clear();
    currentStep = 0;
    saveState();
}

juce::StringArray UndoManager::getUndoTransactionNames() const
{
    juce::StringArray names;

    for (auto i = currentStep - 1; i >= 0; --i)
        names.add (storedStates[i].transactionName);

    return names;
}

juce::StringArray UndoManager::getRedoTransactionNames() const
{
    juce::StringArray names;

    for (auto i = currentStep + 1; i < storedStates.size(); ++i)
        names.add (storedStates[i].transactionName);

    return names;
}

void UndoManager::loadState (const State& stateToLoad)
{
    state.deserialize (stateToLoad.state);
    transactionName = stateToLoad.transactionName;
}

UndoManager::State::State (const juce::ValueTree& tree, const String& name)
{
    state           = tree;
    transactionName = name;
}

UndoManager::ScopedTransaction::ScopedTransaction (UndoManager* um, const String& name)
    : undo (um)
{
    if (undo != nullptr)
        undo->beginNewTransaction (name);
}

UndoManager::ScopedTransaction::~ScopedTransaction()
{
    if (undo != nullptr)
        undo->endTransaction();
}

}  // namespace bav
