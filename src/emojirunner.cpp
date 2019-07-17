/*
   Copyright %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "emojirunner.h"
#include "FileReader.h"

// KF
#include <KLocalizedString>
#include <QtGui/QtGui>

EmojiRunner::EmojiRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("EmojiRunner"));
}

EmojiRunner::~EmojiRunner() = default;

void EmojiRunner::reloadConfiguration() {
    FileReader::readJSONFile();
}

void EmojiRunner::match(Plasma::RunnerContext &context) {
    const QString term = context.query();
    QList<Plasma::QueryMatch> matches;
    Plasma::QueryMatch match(this);
    match.setIconName("kdeapp");
    match.setText("Hello World!");
    matches.append(match);
    context.addMatches(matches);
}

void EmojiRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)
    Q_UNUSED(match)

    // TODO
}

K_EXPORT_PLASMA_RUNNER(emojirunner, EmojiRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "emojirunner.moc"
