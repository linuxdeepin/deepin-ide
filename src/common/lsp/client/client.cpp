// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "client.h"
#include "private/client_p.h"
#include "common/util/custompaths.h"
#include "common/util/processutil.h"
#include "common/unilog.h"

#include <QMetaType>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRunnable>
#include <QThreadPool>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent>
#include <QHostAddress>
#include <QReadWriteLock>

#include <iostream>

#define LANGUAGE_ADAPTER_NAME "languageadapter"
#define LANGUAGE_ADAPTER_PATH CustomPaths::global(CustomPaths::Tools) \
        + QDir::separator() + LANGUAGE_ADAPTER_NAME

namespace newlsp {

static QMutex mutex;
class Client;

Client::Client()
    : d(new ClientPrivate(this))
{
    qRegisterMetaType<lsp::Diagnostics>("lsp::Diagnostics");
    qRegisterMetaType<lsp::SemanticTokensProvider>("lsp::SemanticTokensProvider");
    qRegisterMetaType<lsp::Symbols>("lsp::Symbols");
    qRegisterMetaType<lsp::Locations>("lsp::Locations");
    qRegisterMetaType<lsp::CompletionProvider>("lsp::CompletionProvider");
    qRegisterMetaType<lsp::SignatureHelps>("lsp::SignatureHelps");
    qRegisterMetaType<lsp::DocumentHighlight>("lsp::DocumentHighlight");
    qRegisterMetaType<QList<lsp::Data>>("QList<lsp::Data>");
    qRegisterMetaType<lsp::DefinitionProvider>("lsp::DefinitionProvider");
    qRegisterMetaType<lsp::DiagnosticsParams>("lsp::DiagnosticsParams");
    qRegisterMetaType<lsp::Data>("lsp::Data");
    qRegisterMetaType<lsp::References>("lsp::References");
    qRegisterMetaType<lsp::Position>("lsp::Position");
    qRegisterMetaType<newlsp::Hover>("newlsp::Hover");
    qRegisterMetaType<newlsp::WorkspaceEdit>("newlsp::WorkspaceEdit");
    qRegisterMetaType<newlsp::Position>("newlsp::Position");
    qRegisterMetaType<newlsp::Range>("newlsp::Range");
    qRegisterMetaType<newlsp::PublishDiagnosticsParams>("newlsp::PublishDiagnosticsParams");
    qRegisterMetaType<newlsp::DocumentColorParams>("newlsp::DocumentColorParams");
    qRegisterMetaType<newlsp::DocumentSymbol>("newlsp::DocumentSymbol");
    qRegisterMetaType<newlsp::SymbolInformation>("newlsp::SymbolInformation");
}

Client::~Client()
{
    if (d) {
        delete d;
    }
}

void Client::delta(const SemanticTokensDeltaParams &params)
{
    Q_UNUSED(params);
}

void Client::full(const SemanticTokensParams &params)
{
    Q_UNUSED(params);
}

void Client::range(const SemanticTokensRangeParams &params)
{
    Q_UNUSED(params);
}

void Client::didOpen(const DidOpenTextDocumentParams &params)
{
    Q_UNUSED(params);
}

void Client::didChange(const DidChangeTextDocumentParams &params)
{
    Q_UNUSED(params);
}

void Client::willSave(const WillSaveTextDocumentParams &params)
{
    Q_UNUSED(params);
}

void Client::willSaveWaitUntil(const WillSaveTextDocumentParams &params)
{
    Q_UNUSED(params);
}

void Client::didSave(const DidSaveTextDocumentParams &params)
{
    Q_UNUSED(params);
}

void Client::didClose(const DidCloseTextDocumentParams &params)
{
    Q_UNUSED(params);
}

void Client::declaration(const DeclarationParams &params)
{
    Q_UNUSED(params);
}

void Client::definition(const DefinitionParams &params)
{
    Q_UNUSED(params);
}

void Client::typeDefinition(const TypeDefinitionParams &params)
{
    Q_UNUSED(params);
}

void Client::implementation(const ImplementationParams &params)
{
    Q_UNUSED(params);
}

void Client::references(const ReferenceParams &params)
{
    Q_UNUSED(params);
}

void Client::prepareCallHierarchy(const CallHierarchyPrepareParams &params)
{
    Q_UNUSED(params);
}

void Client::prepareTypeHierarchy(const TypeHierarchyPrepareParams &params)
{
    Q_UNUSED(params);
}

void Client::documentHighlight(const DocumentHighlightParams &params)
{
    Q_UNUSED(params);
}

void Client::documentLink(const DocumentLinkParams &params)
{
    Q_UNUSED(params);
}

void Client::hover(const HoverParams &params)
{
    Q_UNUSED(params);
}

void Client::codeLens(const CodeLensParams &params)
{
    Q_UNUSED(params);
}

void Client::foldingRange(const FoldingRangeParams &params)
{
    Q_UNUSED(params);
}

void Client::selectionRange(const SelectionRangeParams &params)
{
    Q_UNUSED(params);
}

void Client::documentSymbol(const DocumentSymbolParams &params)
{
    Q_UNUSED(params);
}

void Client::inlayHint(const InlayHintParams &params)
{
    Q_UNUSED(params);
}

void Client::inlineValue(const InlineValueParams &params)
{
    Q_UNUSED(params);
}

void Client::moniker(const MonikerParams &params)
{
    Q_UNUSED(params);
}

void Client::completion(const CompletionParams &params)
{
    Q_UNUSED(params);
}

void Client::diagnostic(const DocumentDiagnosticParams &params)
{
    Q_UNUSED(params);
}

void Client::signatureHelp(const SignatureHelpParams &params)
{
    Q_UNUSED(params);
}

void Client::codeAction(const CodeActionParams &params)
{
    Q_UNUSED(params);
}

void Client::documentColor(const DocumentColorParams &params)
{
    d->callMethod("textDocument/documentColor",
                  QJsonDocument::fromJson(
                          QByteArray::fromStdString(toJsonValueStr(params)))
                          .object());
}

void Client::colorPresentation(const ColorPresentationParams &params)
{
    Q_UNUSED(params);
}

void Client::formatting(const DocumentFormattingParams &params)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_FORMATTING,
                  QJsonDocument::fromJson(
                          QByteArray::fromStdString(toJsonValueStr(params)))
                          .object());
}

void Client::rangeFormatting(const QString &filePath, const DocumentRangeFormattingParams &params)
{
    qInfo() << QString::fromStdString(toJsonValueStr(params));
    d->callMethod(lsp::V_TEXTDOCUMENT_RANGEFORMATTING,
                  QJsonDocument::fromJson(
                          QByteArray::fromStdString(toJsonValueStr(params)))
                          .object(),
                  filePath);
}

void Client::onTypeFormatting(const DocumentOnTypeFormattingParams &params)
{
    Q_UNUSED(params);
}

void Client::rename(const RenameParams &params)
{
    Q_UNUSED(params);
}

void Client::prepareRename(const PrepareRenameParams &params)
{
    Q_UNUSED(params);
}

void Client::linkedEditingRange(const LinkedEditingRangeParams &params)
{
    Q_UNUSED(params);
}

void Client::resolve(const CodeAction &codeAction)
{
    Q_UNUSED(codeAction);
}

void Client::resolve(const CompletionItem &params)
{
    Q_UNUSED(params);
}

void Client::resolve() {}

void Client::resolve(const CodeLens &codeLens)
{
    Q_UNUSED(codeLens);
}

void Client::supertypes(const TypeHierarchySupertypesParams &params)
{
    Q_UNUSED(params);
}

void Client::subtypes(const TypeHierarchySubtypesParams &params)
{
    Q_UNUSED(params);
}

void Client::incomingCalls(const CallHierarchyIncomingCallsParams &params)
{
    Q_UNUSED(params);
}

void Client::outgoingCalls(const CallHierarchyOutgoingCallsParams &params)
{
    Q_UNUSED(params);
}

void Client::refresh() {}

void Client::workspace_semanticTokens_refresh() {}

void Client::workspace_inlayHint_refresh() {}

void Client::workspace_inlineValue_refresh() {}

void Client::workspace_diagnostic_refresh() {}

void Client::diagnostic(const WorkspaceDiagnosticParams &params)
{
    Q_UNUSED(params);
}

void Client::selectLspServer(const newlsp::ProjectKey &key)
{
    if (d->proKey == key)
        return;

    d->proKey = key;
    QJsonObject params = newlsp::toQJsonObject(key);
    d->writeLspData(newlsp::notificationData(__FUNCTION__, params).toUtf8());
}

void Client::initRequest(const QString &compile)
{
    QString langQStr = QString::fromStdString(d->proKey.language);
    QString workQStr = QString::fromStdString(d->proKey.workspace);
    d->callMethod(lsp::V_INITIALIZE, lsp::initialize(workQStr, langQStr, compile));
}

void Client::openRequest(const QString &filePath)
{
    d->callNotification(lsp::V_TEXTDOCUMENT_DIDOPEN, lsp::didOpen(filePath));
}

void Client::closeRequest(const QString &filePath)
{
    d->callNotification(lsp::V_TEXTDOCUMENT_DIDCLOSE, lsp::didClose(filePath));
}

void Client::changeRequest(const QString &filePath, const QByteArray &text)
{
    d->callNotification(lsp::V_TEXTDOCUMENT_DIDCHANGE, lsp::didChange(filePath, text, d->fileVersion[filePath]));
}

void Client::symbolRequest(const QString &filePath)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_DOCUMENTSYMBOL, lsp::symbol(filePath), filePath);
}

void Client::renameRequest(const QString &filePath, const lsp::Position &pos, const QString &newName)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_RENAME, lsp::rename(filePath, pos, newName), filePath);
}

void Client::definitionRequest(const QString &filePath, const lsp::Position &pos)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_DEFINITION, lsp::definition(filePath, pos), filePath);
}

void Client::completionRequest(const QString &filePath, const lsp::Position &pos, const lsp::CompletionContext &context)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_COMPLETION, lsp::completion(filePath, pos, context), filePath);
}

void Client::signatureHelpRequest(const QString &filePath, const lsp::Position &pos)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_SIGNATUREHELP, lsp::signatureHelp(filePath, pos), filePath);
}

void Client::referencesRequest(const QString &filePath, const lsp::Position &pos)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_REFERENCES, lsp::references(filePath, pos), filePath);
}

void Client::docHighlightRequest(const QString &filePath, const lsp::Position &pos)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT, lsp::documentHighlight(filePath, pos), filePath);
}

void Client::docSemanticTokensFull(const QString &filePath)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_SEMANTICTOKENS_FULL, lsp::documentSemanticTokensFull(filePath), filePath);
}

void Client::docSemanticTokensRange(const QString &filePath, lsp::Range range)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_SEMANTICTOKENS_RANGE, lsp::documentSemanticTokensRange(filePath, range), filePath);
}

void Client::docHoverRequest(const QString &filePath, const lsp::Position &pos)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_HOVER, lsp::hover(filePath, pos), filePath);
}

void Client::switchHeaderSource(const QString &filePath)
{
    d->callMethod(lsp::V_TEXTDOCUMENT_SWITCHHEADERSOURCE, lsp::switchHeaderSource(filePath));
}

void Client::shutdownRequest()
{
    d->callMethod(lsp::V_SHUTDOWN, lsp::shutdown());
}

void Client::exitRequest()
{
    d->callMethod(lsp::V_EXIT, lsp::exit());
}

void ClientPrivate::callMethod(const QString &method, const QJsonObject &params, const QString &filePath)
{
    requestIndex++;
    requestSave.insert(requestIndex, { method, filePath });
    writeLspData(newlsp::methodData(requestIndex, method, params).toUtf8());
}

void ClientPrivate::callNotification(const QString &method, const QJsonObject &params)
{
    writeLspData(newlsp::notificationData(method, params).toUtf8());
}

void ClientPrivate::writeLspData(const QByteArray &jsonObj)
{
    uniDebug() << "\nclient->server:\n{\n"
               << jsonObj << "\n}";
    q->write(jsonObj);
    q->waitForBytesWritten();
}

bool ClientPrivate::calledError(const QJsonObject &jsonObj)
{
    if (jsonObj.keys().contains(lsp::K_ERROR)) {
        QString errStr = "Failed, called error. code ";
        auto errorObj = jsonObj.value(lsp::K_ERROR).toObject();
        auto calledID = jsonObj.value(K_ID).toInt();
        errStr += QString("%0 ").arg(errorObj.value(lsp::K_CODE).toInt());
        errStr += QString(",%0 ").arg(errorObj.value(lsp::K_MESSAGE).toString());
        if (requestSave.keys().contains(calledID)) {
            auto requestMethod = requestSave.value(calledID).method;
            errStr += QString("from: %0").arg(requestMethod);
        }
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::initResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_INITIALIZE) {
        requestSave.remove(calledID);

        QJsonObject semanticTokensProviderObj = jsonObj.value("result").toObject().value("capabilities").toObject().value("semanticTokensProvider").toObject();
        QJsonObject fullObj = semanticTokensProviderObj.value("full").toObject();
        QJsonObject legendObj = semanticTokensProviderObj.value("legend").toObject();

        lsp::SemanticTokensProvider provider {
            lsp::SemanticTokensProvider::Full {
                    fullObj.value("delta").toBool() },
            lsp::SemanticTokensProvider::Legend {
                    cvtStringList(legendObj.value("tokenTypes").toArray()),
                    cvtStringList(legendObj.value("tokenModifiers").toArray()) },
            semanticTokensProviderObj.value("range").toBool()
        };

        secTokensProvider = provider;

        emit q->requestResult(provider);
        return true;
    }
    return false;
}

bool ClientPrivate::openResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_DIDOPEN) {
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::changeResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_DIDCHANGE) {
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::symbolResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_DOCUMENTSYMBOL) {
        auto filePath = requestSave.value(calledID).file;
        requestSave.remove(calledID);

        QList<DocumentSymbol> docSymbols;
        QList<SymbolInformation> symbolInfos;
        auto value = jsonObj.value(K_RESULT);
        if (value.isArray()) {
            auto array = value.toArray();
            if (!array.isEmpty()) {
                auto arrayObj = array.first().toObject();
                if (arrayObj.contains("range")) {
                    docSymbols = parseDocumentSymbol(array);
                } else {
                    symbolInfos = parseDocumentSymbolInfo(array);
                }
            }
        }

        emit q->symbolResult(docSymbols, filePath);
        emit q->symbolResult(symbolInfos, filePath);
        return true;
    }
    return false;
}

bool ClientPrivate::renameResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_RENAME) {
        requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value(K_RESULT).toObject();

        QJsonObject changesObj = resultObj.value("changes").toObject();
        newlsp::WorkspaceEdit workspaceEdit;
        if (!changesObj.isEmpty()) {
            // std::optional<> changes;
            newlsp::WorkspaceEdit::Changes changes;
            for (auto fileKey : changesObj.keys()) {
                auto addionTextEditArray = changesObj[fileKey].toArray();
                std::vector<newlsp::TextEdit> textEdits;
                for (auto addion : addionTextEditArray) {
                    auto addionObj = addion.toObject();
                    auto rangeObj = addionObj.value(lsp::K_RANGE).toObject();
                    auto startObj = rangeObj.value(lsp::K_START).toObject();
                    auto endObj = rangeObj.value(lsp::K_END).toObject();
                    std::string newText = addionObj.value(lsp::K_NewText).toString().toStdString();
                    newlsp::Position startPos { startObj.value(lsp::K_LINE).toInt(), startObj.value(lsp::K_CHARACTER).toInt() };
                    newlsp::Position endPos { endObj.value(lsp::K_LINE).toInt(), endObj.value(lsp::K_CHARACTER).toInt() };
                    newlsp::Range range { startPos, endPos };
                    textEdits.push_back(newlsp::TextEdit { range, newText });
                }
                changes[fileKey.toStdString()] = textEdits;
            }
            workspaceEdit.changes = changes;
        }

        QJsonArray documentChangesArray = resultObj.value("documentChanges").toArray();
        if (!documentChangesArray.isEmpty()) {
            newlsp::WorkspaceEdit::DocumentChanges documentChanges;
            std::vector<newlsp::TextDocumentEdit> textDocumentEdits;
            std::vector<newlsp::CreateFile> createFiles;
            std::vector<newlsp::RenameFile> renameFiles;
            std::vector<newlsp::DeleteFile> deleteFiles;
            for (auto one : documentChangesArray) {
                QJsonObject oneObj = one.toObject();
                if (oneObj.contains("edits") && oneObj.contains("textDocument")) {   // std::vector<newlsp::TextDocumentEdit>
                    newlsp::TextDocumentEdit textDocumentEdit;

                    newlsp::OptionalVersionedTextDocumentIdentifier ovtdi;
                    QJsonObject textDocumentOneObj = oneObj.value("textDocument").toObject();
                    ovtdi.version = textDocumentOneObj.value(lsp::K_VERSION).toInt();
                    ovtdi.uri = textDocumentOneObj.value(lsp::K_URI).toString().toStdString();
                    textDocumentEdit.textDocument.version = ovtdi.version;
                    textDocumentEdit.textDocument.uri = ovtdi.uri;

                    std::vector<newlsp::AnnotatedTextEdit> annotatedTextEdits;
                    std::vector<newlsp::TextEdit> textEdits;
                    QJsonArray editsArray = oneObj.value("edits").toArray();
                    for (auto editsOne : editsArray) {
                        QJsonObject editsOneObj = editsOne.toObject();
                        QJsonObject editsOneRangeObj = editsOneObj.value("range").toObject();
                        QJsonObject editsOneRangeStartObj = editsOneRangeObj.value(lsp::K_START).toObject();
                        QJsonObject editsOneRangeEndObj = editsOneRangeObj.value(lsp::K_END).toObject();
                        std::string editsOneNewText = editsOneObj.value("newText").toString().toStdString();
                        newlsp::Range editsOneRange {
                            newlsp::Position { editsOneRangeStartObj.value(lsp::K_LINE).toInt(), editsOneRangeStartObj.value(lsp::K_CHARACTER).toInt() },
                            newlsp::Position { editsOneRangeEndObj.value(lsp::K_LINE).toInt(), editsOneRangeEndObj.value(lsp::K_CHARACTER).toInt() }
                        };
                        if (editsOneObj.contains("annotationId")) {   // edits: (TextEdit | AnnotatedTextEdit)[];
                            newlsp::ChangeAnnotationIdentifier changeAnnIdf = editsOneObj.value("annotationId").toString().toStdString();
                            newlsp::AnnotatedTextEdit annotatedTextEdit {};
                            annotatedTextEdit.range = editsOneRange;
                            annotatedTextEdit.newText = editsOneNewText;
                            annotatedTextEdit.annotationId = changeAnnIdf;
                            annotatedTextEdits.push_back(annotatedTextEdit);
                        } else {
                            newlsp::TextEdit textEdit;
                            textEdit.range = editsOneRange;
                            textEdit.newText = editsOneNewText;
                            textEdits.push_back(textEdit);
                        }
                    }
                    if (!annotatedTextEdits.empty()) {
                        textDocumentEdit.edits = annotatedTextEdits;
                    } else if (!textEdits.empty()) {
                        textDocumentEdit.edits = textEdits;
                    }
                    textDocumentEdits.push_back(textDocumentEdit);
                } else {
                    QString oneObjKind = oneObj.value("kind").toString();
                    if ("create" == oneObjKind) {
                        newlsp::CreateFile createFile;
                        createFile.uri = oneObj.value("uri").toString().toStdString();
                        QJsonObject oneObjOptions = oneObj.value("options").toObject();
                        if (!oneObjOptions.empty()) {
                            newlsp::CreateFileOptions options;
                            options.overwrite = oneObjOptions.value("overwrite").toBool();
                            options.ignoreIfExists = oneObjOptions.value("ignoreIfExists").toBool();
                            createFile.options = options;
                        }
                        QJsonValue annotationIdJV = oneObj.value("annotationId");
                        if (!annotationIdJV.isNull()) {
                            createFile.annotationId = annotationIdJV.toString().toStdString();
                        }
                        createFiles.push_back(createFile);
                    } else if ("delete" == oneObjKind) {
                        newlsp::DeleteFile deleteFile;
                        deleteFile.uri = oneObj.value("uri").toString().toStdString();
                        QJsonObject oneObjOptions = oneObj.value("options").toObject();
                        if (!oneObjOptions.empty()) {
                            newlsp::DeleteFileOptions options;
                            options.recursive = oneObjOptions.value("recursive").toBool();
                            options.ignoreIfNotExists = oneObjOptions.value("ignoreIfNotExists").toBool();
                            deleteFile.options = options;
                        }
                        QJsonValue annotationIdJV = oneObj.value("annotationId");
                        if (!annotationIdJV.isNull()) {
                            deleteFile.annotationId = annotationIdJV.toString().toStdString();
                        }
                        deleteFiles.push_back(deleteFile);
                    } else if ("rename" == oneObjKind) {
                        newlsp::RenameFile renameFile;
                        renameFile.oldUri = oneObj.value("oldUri").toString().toStdString();
                        renameFile.newUri = oneObj.value("newUri").toString().toStdString();
                        QJsonObject oneObjOptions = oneObj.value("options").toObject();
                        if (!oneObjOptions.empty()) {
                            newlsp::RenameFileOptions options;
                            options.overwrite = oneObjOptions.value("overwrite").toBool();
                            options.ignoreIfExists = oneObjOptions.value("ignoreIfExists").toBool();
                            renameFile.options = options;
                        }
                        QJsonValue annotationIdJV = oneObj.value("annotationId");
                        if (!annotationIdJV.isNull()) {
                            renameFile.annotationId = annotationIdJV.toString().toStdString();
                        }
                        renameFiles.push_back(renameFile);
                    }
                }
                // set workspaceEdit.documentChanges
                if (!textDocumentEdits.empty()) {
                    workspaceEdit.documentChanges = textDocumentEdits;
                } else if (!createFiles.empty()) {
                    workspaceEdit.documentChanges = createFiles;
                } else if (!renameFiles.empty()) {
                    workspaceEdit.documentChanges = renameFiles;
                } else if (!deleteFiles.empty()) {
                    workspaceEdit.documentChanges = deleteFiles;
                }
            }
        }

        QJsonObject changeAnnotationsObj = resultObj.value("changeAnnotations").toObject();
        if (!changeAnnotationsObj.isEmpty()) {
            newlsp::WorkspaceEdit::ChangeAnnotations changeAnnotations;
            for (auto idKey : changeAnnotationsObj.keys()) {
                QJsonObject changeAnnotationObj = changeAnnotationsObj[idKey].toObject();
                newlsp::ChangeAnnotation changeAnnotation;
                std::string label = changeAnnotationObj.value("label").toString().toStdString();
                changeAnnotation.label = label;
                if (changeAnnotationObj.contains("needsConfirmation")) {
                    changeAnnotation.needsConfirmation = changeAnnotationObj.value("needsConfirmation").toBool();
                }
                if (changeAnnotationObj.contains("description")) {
                    changeAnnotation.description = changeAnnotationObj.value("description").toString().toStdString();
                }
                changeAnnotations[idKey.toStdString()] = changeAnnotation;
            }
            workspaceEdit.changeAnnotations = changeAnnotations;
        }
        emit q->renameRes(workspaceEdit);
        return true;
    }
    return false;
}

bool ClientPrivate::rangeFormattingResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_RANGEFORMATTING) {
        auto filePath = requestSave.value(calledID).file;
        requestSave.remove(calledID);

        QJsonValue resultJV = jsonObj.value(K_RESULT);
        if (resultJV.isArray()) {
            QJsonArray resultArray = resultJV.toArray();
            if (resultArray.count() <= 0) {
                return false;
            }
            std::vector<TextEdit> edits;
            for (auto one : resultArray) {
                QJsonObject oneObj = one.toObject();
                TextEdit edit;
                edit.newText = oneObj.value(lsp::K_NewText).toString().toStdString();
                QJsonObject rangeObj = oneObj.value(lsp::K_RANGE).toObject();
                QJsonObject startObj = rangeObj.value(lsp::K_START).toObject();
                QJsonObject endObj = rangeObj.value(lsp::K_END).toObject();
                edit.range.start = { startObj.value(lsp::K_LINE).toInt(),
                                     startObj.value(lsp::K_CHARACTER).toInt() };
                edit.range.end = { endObj.value(lsp::K_LINE).toInt(),
                                   endObj.value(lsp::K_CHARACTER).toInt() };
                edits.push_back(edit);
            }
            emit q->rangeFormattingRes(edits, filePath);
        }
    }
    return false;
}

bool ClientPrivate::definitionResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_DEFINITION) {
        auto filePath = requestSave.value(calledID).file;
        requestSave.remove(calledID);

        QJsonValue resultJV = jsonObj.value(K_RESULT);
        if (resultJV.isArray()) {
            QJsonArray resultArray = resultJV.toArray();
            if (resultArray.count() <= 0) {
                return false;
            }
            std::vector<newlsp::Location> locations;
            std::vector<newlsp::LocationLink> locationLinks;
            for (auto one : resultArray) {
                QJsonObject oneObj = one.toObject();
                if (oneObj.contains("range") && oneObj.contains("uri")) {
                    QJsonObject rangeObj = oneObj.value("range").toObject();
                    QJsonObject startObj = rangeObj.value("start").toObject();
                    QJsonObject endObj = rangeObj.value("end").toObject();
                    std::string uri = oneObj.value("uri").toString().toStdString();
                    newlsp::Range range {
                        { startObj.value("line").toInt(), startObj.value("character").toInt() },
                        { endObj.value("line").toInt(), endObj.value("character").toInt() }
                    };
                    locations.push_back({ uri, range });
                } else if (oneObj.contains("originSelectionRange")
                           && oneObj.contains("targetUri")
                           && oneObj.contains("targetRange")
                           && oneObj.contains("targetSelectionRange")) {
                    std::string targetUri = oneObj.value("targetUri").toString().toStdString();
                    QJsonObject rangeObj, startObj, endObj;
                    // originSelectionRange
                    rangeObj = oneObj.value("originSelectionRange").toObject();
                    startObj = rangeObj.value("start").toObject();
                    endObj = endObj.value("end").toObject();
                    newlsp::Range originSelectionRange {
                        { startObj.value("line").toInt(), startObj.value("character").toInt() },
                        { endObj.value("line").toInt(), endObj.value("character").toInt() }
                    };
                    // targetRange
                    rangeObj = oneObj.value("targetRange").toObject();
                    startObj = rangeObj.value("start").toObject();
                    endObj = endObj.value("end").toObject();
                    newlsp::Range targetRange {
                        { startObj.value("line").toInt(), startObj.value("character").toInt() },
                        { endObj.value("line").toInt(), endObj.value("character").toInt() }
                    };
                    // targetSelectionRange
                    rangeObj = oneObj.value("targetSelectionRange").toObject();
                    startObj = rangeObj.value("start").toObject();
                    endObj = endObj.value("end").toObject();
                    newlsp::Range targetSelectionRange {
                        { startObj.value("line").toInt(), startObj.value("character").toInt() },
                        { endObj.value("line").toInt(), endObj.value("character").toInt() }
                    };
                    locationLinks.push_back({ originSelectionRange, targetUri, targetRange, targetSelectionRange });
                }
            }

            if (!locationLinks.empty()) {
                emit q->definitionRes(locationLinks, filePath);
                return true;
            } else if (!locations.empty()) {
                emit q->definitionRes(locations, filePath);
                return true;
            }
        } else if (resultJV.isObject()) {
            QJsonObject locationObj = resultJV.toObject();
            if (locationObj.contains("range") && locationObj.contains("uri")) {
                QJsonObject rangeObj = locationObj.value("range").toObject();
                QJsonObject startObj = rangeObj.value("start").toObject();
                QJsonObject endObj = rangeObj.value("end").toObject();
                std::string uri = locationObj.value("uri").toString().toStdString();
                newlsp::Range range {
                    { startObj.value("line").toInt(), startObj.value("character").toInt() },
                    { endObj.value("line").toInt(), endObj.value("character").toInt() }
                };
                emit q->definitionRes(newlsp::Location { uri, range }, filePath);
                return true;
            }
        }
    }
    return false;
}

bool ClientPrivate::completionResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_COMPLETION) {
        requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value(K_RESULT).toObject();
        QJsonArray itemsArray = resultObj.value("items").toArray();
        lsp::CompletionProvider completionProvider;
        lsp::CompletionItems items;
        for (auto item : itemsArray) {
            QJsonObject itemObj = item.toObject();
            QJsonArray editsArray = itemObj.value("additionalTextEdits").toArray();
            lsp::AdditionalTextEdits additionalTextEdits;
            for (auto edit : editsArray) {
                QJsonObject textEditObj = edit.toObject();
                QString newText = textEditObj.value("newText").toString();
                QJsonObject rangeObj = textEditObj.value("range").toObject();
                QJsonObject startObj = rangeObj.value(lsp::K_START).toObject();
                QJsonObject endObj = rangeObj.value(lsp::K_END).toObject();
                lsp::Position start { startObj.value(lsp::K_LINE).toInt(), startObj.value(lsp::K_CHARACTER).toInt() };
                lsp::Position end { endObj.value(lsp::K_LINE).toInt(), endObj.value(lsp::K_CHARACTER).toInt() };
                additionalTextEdits << lsp::TextEdit { newText, lsp::Range { start, end } };
            }

            QJsonObject documentationObj = itemObj.value("documentation").toObject();
            struct lsp::Documentation documentation
            {
                documentationObj.value("kind").toString(), documentationObj.value("value").toString()
            };

            QJsonObject textEditObj = itemObj.value("textEdit").toObject();
            QJsonObject textEditRangeObj = textEditObj.value("range").toObject();
            QJsonObject textEditStartObj = textEditRangeObj.value(lsp::K_START).toObject();
            QJsonObject textEditEndObj = textEditRangeObj.value(lsp::K_END).toObject();
            QString newText = textEditObj.value("newText").toString();
            lsp::Position start { textEditStartObj.value(lsp::K_LINE).toInt(), textEditStartObj.value(lsp::K_CHARACTER).toInt() };
            lsp::Position end { textEditEndObj.value(lsp::K_LINE).toInt(), textEditEndObj.value(lsp::K_CHARACTER).toInt() };
            lsp::TextEdit textEdit { newText, lsp::Range { start, end } };

            items << lsp::CompletionItem {
                additionalTextEdits,
                documentation,
                itemObj.value("filterText").toString(),
                itemObj.value("insertText").toString(),
                (lsp::InsertTextFormat)itemObj.value("insertTextFormat").toInt(),
                (lsp::CompletionItem::Kind)(itemObj.value("kind").toInt()),
                itemObj.value("label").toString(),
                itemObj.value("detail").toString(),
                itemObj.value("score").toDouble(),
                itemObj.value("sortText").toString(),
                textEdit
            };
        }

        completionProvider.items = items;
        completionProvider.isIncomplete = resultObj.value("isIncomplete").toBool();

        emit q->requestResult(completionProvider);
        return true;
    }
    return false;
}

bool ClientPrivate::signatureHelpResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_SIGNATUREHELP) {
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::hoverResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_HOVER) {
        requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value("result").toObject();
        newlsp::Hover hover;
        QJsonValue contentsJV = resultObj.value("contents");
        if (contentsJV.isArray()) {   // to MarkedString[]
            QJsonArray contentsJA = contentsJV.toArray();
            std::vector<newlsp::MarkedString> markedStringVec {};
            for (auto one : contentsJA) {
                if (one.isObject()) {
                    QJsonObject oneObj = one.toObject();
                    markedStringVec.push_back(newlsp::MarkedString {
                            oneObj.value("language").toString().toStdString(),
                            oneObj.value("value").toString().toStdString() });
                } else if (one.isString()) {
                    markedStringVec.push_back(newlsp::MarkedString { one.toString().toStdString() });
                }
            }
            hover.contents = markedStringVec;
        } else {   // MarkedString or MarkupContent
            QJsonObject contentsObj = contentsJV.toObject();
            QJsonValue contents_kind_JV = contentsObj.value("kind");
            QJsonValue contents_value_JV = contentsObj.value("value");
            QJsonValue contents_language_JV = contentsObj.value("language");
            if (!contents_kind_JV.isNull() && !contents_value_JV.isNull()) {   // MarkupContent
                hover.contents = newlsp::MarkupContent {
                    contents_kind_JV.toString().toStdString(),
                    contents_value_JV.toString().toStdString()
                };
            } else if (!contents_language_JV.isNull() && !contents_value_JV.isNull()) {   // MarkupString
                hover.contents = newlsp::MarkedString {
                    contents_language_JV.toString().toStdString(),
                    contents_value_JV.toString().toStdString()
                };
            } else {
                hover.contents = newlsp::MarkedString { contentsJV.toString().toStdString() };
            }
        }
        QJsonValue rangeJV = resultObj.value("range");
        if (!rangeJV.isNull()) {
            QJsonObject rangeObj = rangeJV.toObject();
            QJsonObject startObj = rangeObj.value(lsp::K_START).toObject();
            QJsonObject endObj = rangeObj.value(lsp::K_END).toObject();
            hover.range = newlsp::Range {
                newlsp::Position { startObj.value(lsp::K_LINE).toInt(), startObj.value(lsp::K_CHARACTER).toInt() },
                newlsp::Position { endObj.value(lsp::K_LINE).toInt(), endObj.value(lsp::K_CHARACTER).toInt() }
            };
        }
        emit q->hoverRes(hover);
        return true;
    }
    return false;
}

bool ClientPrivate::referencesResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_REFERENCES) {
        lsp::References refs;
        auto resultArray = jsonObj.value(K_RESULT).toArray();
        for (auto item : resultArray) {
            auto itemObj = item.toObject();
            auto rangeObj = itemObj.value(lsp::K_RANGE).toObject();
            auto startObj = rangeObj.value(lsp::K_START).toObject();
            auto endObj = rangeObj.value(lsp::K_END).toObject();
            QString url = itemObj.value(lsp::K_URI).toString();
            lsp::Location location;
            location.fileUrl = url;
            location.range.start = lsp::Position { startObj.value(lsp::K_LINE).toInt(),
                                                   startObj.value(lsp::K_CHARACTER).toInt() };
            location.range.end = lsp::Position { endObj.value(lsp::K_LINE).toInt(),
                                                 endObj.value(lsp::K_CHARACTER).toInt() };
            refs << location;
        }
        emit q->requestResult(refs);
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::docHighlightResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT) {
        auto filePath = requestSave.value(calledID).file;
        requestSave.remove(calledID);

        auto result = jsonObj.value(K_RESULT);
        QList<DocumentHighlight> docHighlightList;
        if (result.isArray())
            docHighlightList = parseDocumentHighlight(result.toArray());

        emit q->documentHighlightResult(docHighlightList, filePath);
        return true;
    }
    return false;
}

bool ClientPrivate::docSemanticTokensFullResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_SEMANTICTOKENS + "/full") {
        auto filePath = requestSave.value(calledID).file;
        requestSave.remove(calledID);

        QJsonObject result = jsonObj.value(K_RESULT).toObject();
        semanticTokenResultId = jsonObj.value("resultId").toInt();

        QJsonArray dataArray = result.value(lsp::K_DATA).toArray();
        if (dataArray.isEmpty())
            return true;

        QList<lsp::Data> results;
        auto itera = dataArray.begin();
        while (itera != dataArray.end()) {
            results << lsp::Data {
                lsp::Position { itera++->toInt(), itera++->toInt() },
                int(itera++->toInt()),
                itera++->toInt(),
                lsp::fromTokenModifiers(itera++->toInt())
            };
        }
        emit q->requestResult(results, filePath);
        return true;
    }
    return false;
}

bool ClientPrivate::closeResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_DIDCLOSE) {
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::exitResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_EXIT) {
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::switchHeaderSourceResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_TEXTDOCUMENT_SWITCHHEADERSOURCE) {
        requestSave.remove(calledID);

        auto ret = jsonObj.value(K_RESULT).toString();
        QUrl url(ret);
        emit q->switchHeaderSourceResult(url.toLocalFile());
        return true;
    }

    return false;
}

bool ClientPrivate::shutdownResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (requestSave.keys().contains(calledID)
        && requestSave.value(calledID).method == lsp::V_SHUTDOWN) {
        requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool ClientPrivate::diagnosticsCalled(const QJsonObject &jsonObj)
{
    if (!jsonObj.keys().contains(K_METHOD)
        || jsonObj.value(K_METHOD).toString() != lsp::V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS)
        return false;

    newlsp::PublishDiagnosticsParams publishDiagnosticsParams;

    QJsonObject paramsObj = jsonObj.value(K_PARAMS).toObject();
    QJsonArray array = paramsObj.value(lsp::K_DIAGNOSTICS).toArray();

    for (auto val : array) {
        QJsonObject diagnosticObj = val.toObject();
        QJsonObject rangeObj = diagnosticObj.value(lsp::K_RANGE).toObject();
        QJsonObject startObj = rangeObj.value(lsp::K_START).toObject();
        QJsonObject endObj = rangeObj.value(lsp::K_END).toObject();
        std::vector<newlsp::DiagnosticRelatedInformation> reletedInformation;
        for (auto reInfo : diagnosticObj.value(lsp::K_RELATEDINFOMATION).toArray()) {
            auto reInfoObj = reInfo.toObject();
            QJsonObject reInfoLocationObj = reInfoObj.value(lsp::K_LOCATION).toObject();
            QJsonObject reInfoLocationRangeObj = reInfoLocationObj.value(lsp::K_RANGE).toObject();
            QJsonObject reInfoLocationStartObj = reInfoLocationRangeObj.value(lsp::K_START).toObject();
            QJsonObject reInfoLocationEndObj = reInfoLocationRangeObj.value(lsp::K_END).toObject();
            std::string reInfoLocationUrl = reInfoLocationObj.value(lsp::K_URI).toString().toStdString();
            std::string reInfoMessage = reInfoObj.value(lsp::K_MESSAGE).toString().toStdString();
            newlsp::DiagnosticRelatedInformation infomationOne {
                newlsp::Location {
                        newlsp::DocumentUri { reInfoLocationUrl },
                        newlsp::Range {
                                { reInfoLocationRangeObj.value(lsp::K_LINE).toInt(), reInfoLocationRangeObj.value(lsp::K_CHARACTER).toInt() },
                                { reInfoLocationEndObj.value(lsp::K_LINE).toInt(), reInfoLocationEndObj.value(lsp::K_CHARACTER).toInt() } } },
                std::string {
                        reInfoMessage }
            };
            reletedInformation.push_back(infomationOne);
        }

        newlsp::Diagnostic diagnostic;
        diagnostic.range = {
            { startObj.value(lsp::K_LINE).toInt(), startObj.value(lsp::K_CHARACTER).toInt() },
            { endObj.value(lsp::K_LINE).toInt(), endObj.value(lsp::K_CHARACTER).toInt() }
        };

        QJsonValue severityJV = diagnosticObj.value(lsp::K_SEVERITY);
        if (!severityJV.isNull()) {
            diagnostic.severity = lsp::Diagnostic::Severity(severityJV.toInt());
        }

        QJsonValue codeJV = diagnosticObj.value(lsp::K_CODE);
        if (!codeJV.isNull()) {
            if (codeJV.isString()) {
                diagnostic.code = codeJV.toString().toStdString();
            } else {
                diagnostic.code = int(codeJV.toInt());
            }
        }

        QJsonValue codeDescriptionJV = diagnosticObj.value("codeDescription");
        if (!codeDescriptionJV.isNull()) {
            QJsonObject cdObj = codeDescriptionJV.toObject();
            diagnostic.codeDescription = { cdObj.value("href").toString().toStdString() };
        }

        QJsonValue sourceJV = diagnosticObj.value("source");
        if (!sourceJV.isNull()) {
            diagnostic.source = sourceJV.toString().toStdString();
        }

        QJsonValue messageJV = diagnosticObj.value(lsp::K_MESSAGE);
        if (!messageJV.isNull()) {
            diagnostic.message = messageJV.toString().toStdString();
        }

        QJsonValue tagsJV = diagnosticObj.value("tags");
        if (!tagsJV.isNull()) {
            std::vector<Enum::DiagnosticTag::type_value> tags;
            if (tagsJV.isArray()) {
                QJsonArray tagsArray = tagsJV.toArray();
                for (auto one : tagsArray) {
                    tags.push_back(one.toInt());
                }
            }
            diagnostic.tags = tags;
        }

        if (!reletedInformation.empty()) {
            diagnostic.relatedInformation = reletedInformation;
        }

        QJsonValue dataJV = diagnosticObj.value("data");
        if (!dataJV.isNull()) {
            //nothing to do
        }
        publishDiagnosticsParams.diagnostics.push_back(diagnostic);
    }

    publishDiagnosticsParams.version = paramsObj.value(lsp::K_VERSION).toInt();
    publishDiagnosticsParams.uri = paramsObj.value(lsp::K_URI).toString().toStdString();
    emit q->publishDiagnostics(publishDiagnosticsParams);
    return true;
}

QList<DocumentSymbol> ClientPrivate::parseDocumentSymbol(const QJsonArray &array)
{
    QList<DocumentSymbol> docSymbols;
    for (const auto &value : array) {
        if (!value.isObject())
            continue;

        auto obj = value.toObject();
        DocumentSymbol symbol;
        symbol.name = obj.value("name").toString();
        symbol.kind = obj.value("kind").toInt();
        symbol.range = parseRange(obj.value("range").toObject());
        symbol.selectionRange = parseRange(obj.value("selectionRange").toObject());
        symbol.children = parseDocumentSymbol(obj.value("children").toArray());

        auto val = obj.value("detail");
        symbol.detail = val.isUndefined() ? std::nullopt : std::make_optional(val.toString());
        val = obj.value("deprecated");
        symbol.deprecated = val.isUndefined() ? std::nullopt : std::make_optional(val.toBool());

        docSymbols.append(symbol);
    }

    return docSymbols;
}

QList<SymbolInformation> ClientPrivate::parseDocumentSymbolInfo(const QJsonArray &array)
{
    QList<SymbolInformation> symbolInfos;
    for (const auto &value : array) {
        if (!value.isObject())
            continue;

        auto obj = value.toObject();
        SymbolInformation info;
        info.name = obj.value("name").toString();
        info.kind = obj.value("kind").toInt();
        info.location = parseLocation(obj.value("location").toObject());

        auto val = obj.value("deprecated");
        info.deprecated = val.isUndefined() ? std::nullopt : std::make_optional(val.toBool());
        val = obj.value("containerName");
        info.containerName = val.isUndefined() ? std::nullopt : std::make_optional(val.toString());

        symbolInfos.append(info);
    }

    return symbolInfos;
}

QList<DocumentHighlight> ClientPrivate::parseDocumentHighlight(const QJsonArray &array)
{
    QList<DocumentHighlight> highlightList;
    for (const auto &value : array) {
        if (!value.isObject())
            continue;

        DocumentHighlight dh;
        auto obj = value.toObject();
        auto val = obj.value("kind");
        dh.kind = value.isUndefined() ? std::nullopt : std::make_optional(val.toInt());
        dh.range = parseRange(obj.value("range").toObject());

        highlightList.append(dh);
    }

    return highlightList;
}

Range ClientPrivate::parseRange(const QJsonObject &obj)
{
    Range range;
    auto iter = obj.begin();
    for (; iter != obj.end(); ++iter) {
        const auto &valObj = iter.value().toObject();
        int line = valObj.value("line").toInt();
        int character = valObj.value("character").toInt();
        if (iter.key() == "start") {
            range.start.line = line;
            range.start.character = character;
        } else if (iter.key() == "end") {
            range.end.line = line;
            range.end.character = character;
        }
    }

    return range;
}

Location ClientPrivate::parseLocation(const QJsonObject &obj)
{
    Location location;
    location.range = parseRange(obj.value("range").toObject());
    location.uri = obj.value("uri").toString().toStdString();

    return location;
}

bool ClientPrivate::serverCalled(const QJsonObject &jsonObj)
{
    if (diagnosticsCalled(jsonObj))
        return true;

    return false;
}

bool ClientPrivate::calledResult(const QJsonObject &jsonObj)
{
    int calledID = jsonObj.value(K_ID).toInt();
    if (!requestSave.keys().contains(calledID)
        || !jsonObj.contains(K_RESULT))
        return false;

    bool any = false;
    any |= initResult(jsonObj);
    any |= openResult(jsonObj);
    any |= symbolResult(jsonObj);
    any |= definitionResult(jsonObj);
    any |= referencesResult(jsonObj);
    any |= renameResult(jsonObj);
    any |= completionResult(jsonObj);
    any |= signatureHelpResult(jsonObj);
    any |= hoverResult(jsonObj);
    any |= docHighlightResult(jsonObj);
    any |= docSemanticTokensFullResult(jsonObj);
    any |= closeResult(jsonObj);
    any |= shutdownResult(jsonObj);
    any |= exitResult(jsonObj);
    any |= rangeFormattingResult(jsonObj);
    any |= switchHeaderSourceResult(jsonObj);

    requestSave.remove(calledID);

    return any;
}

void ClientPrivate::doReadStdoutLine()
{
    while (q->canReadLine()) {
        doReadedLine(q->readLine());
    }
    if (q->bytesAvailable()) {
        doReadedLine(q->readAllStandardOutput());
    }
}

void ClientPrivate::identifyJsonObject(const QJsonObject &jsonObj)
{
    if (calledError(jsonObj)) {
        uniCritical() << "\nclient <- server:\n{\n"
                      << jsonObj << "\n}";
        return;
    }

    if (calledResult(jsonObj)) {
        uniWarning() << "\nclient <- server:\n{\n"
                     << jsonObj << "\n}";
        return;
    }

    if (serverCalled(jsonObj)) {
        uniWarning() << "\nclient <- server:\n{\n"
                     << jsonObj << "\n}";
        return;
    }
}

lsp::SemanticTokensProvider Client::initSecTokensProvider()
{
    return d->secTokensProvider;
}

ClientPrivate::ClientPrivate(Client *const q)
    : newlsp::StdoutJsonRpcParser(), q(q), requestIndex(0), requestSave({}), semanticTokenResultId(0), fileVersion({}), secTokensProvider({}), proKey({})
{

    q->setProgram(LANGUAGE_ADAPTER_PATH);
    q->setArguments({ "--parentPid", QString::number(qApp->applicationPid()) });

    QObject::connect(q, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [&](int exitCode, QProcess::ExitStatus status) {
                         uniInfo() << "\nclient <- server\n{\n"
                                   << "lspServerProc finished:\n"
                                   << exitCode << status << "\n}";
                     });

    QObject::connect(q, &QProcess::readyReadStandardError,
                     this, [=]() {
                         uniCritical() << "\nclient <- server\n{\n"
                                       << q->readAllStandardError() << "\n}";
                     });

    QObject::connect(q, &QProcess::readyReadStandardOutput,
                     this, &ClientPrivate::doReadStdoutLine,
                     Qt::DirectConnection);

    if (q->state() == QProcess::NotRunning) {
        q->start();
        q->waitForStarted();
    }

    QObject::connect(this, &StdoutJsonRpcParser::readedJsonObject,
                     this, &ClientPrivate::identifyJsonObject);
}

QStringList ClientPrivate::cvtStringList(const QJsonArray &array)
{
    QStringList ret;
    for (auto val : array) {
        ret << val.toString();
    }
    return ret;
}

}   // namespace newlsp
