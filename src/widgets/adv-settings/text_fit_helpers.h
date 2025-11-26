#ifndef TEXT_FIT_HELPERS_H
#define TEXT_FIT_HELPERS_H

#include <QWidget>
#include <QFont>
#include <QFontMetrics>
#include <QPushButton>
#include <QLabel>
#include <QAbstractButton>
#include <QGroupBox>
#include <QLayout>
#include <QRegularExpression>
#include <QList>
#include <QMap>

// Calculate required width for text in a widget
static int calculateRequiredTextWidth(QWidget* widget)
{
    if (!widget) return 0;

    QString text;
    int contentMargins = 0;

    // Get text based on widget type
    if (auto* button = qobject_cast<QPushButton*>(widget)) {
        text = button->text();
        contentMargins = 6; // Buttons have padding
    }
    else if (auto* label = qobject_cast<QLabel*>(widget)) {
        text = label->text();
        contentMargins = 4;
    }
    else if (auto* abstractBtn = qobject_cast<QAbstractButton*>(widget)) {
        text = abstractBtn->text();
        contentMargins = 6;
    }
    else {
        return 0;
    }

    if (text.isEmpty()) return 0;

    // Remove HTML tags for width calculation
    QString plainText = text;
    plainText.remove(QRegularExpression("<[^>]*>"));

    QFont currentFont = widget->font();
    QFontMetrics fm(currentFont);
    int textWidth = fm.horizontalAdvance(plainText);

    return textWidth + contentMargins + 4; // Add small extra padding
}

// Adjust all buttons in a group box to the same width based on longest text
static void adjustButtonGroupToFit(QGroupBox* groupBox)
{
    if (!groupBox) return;

    // Find all buttons in this group box
    QList<QPushButton*> buttons;
    for (QObject* child : groupBox->children()) {
        if (QPushButton* button = qobject_cast<QPushButton*>(child)) {
            buttons.append(button);
        }
    }

    if (buttons.isEmpty()) return;

    // Reset all button size constraints first to allow shrinking
    for (QPushButton* button : buttons) {
        button->setMinimumWidth(0);
        button->setMaximumWidth(QWIDGETSIZE_MAX);
        button->adjustSize();
    }

    // Find maximum required width
    int maxRequiredWidth = 0;
    for (QPushButton* button : buttons) {
        int required = calculateRequiredTextWidth(button);
        if (required > maxRequiredWidth) {
            maxRequiredWidth = required;
        }
    }

    if (maxRequiredWidth == 0) return;

    // Set all buttons to the minimum width (allows flexibility)
    for (QPushButton* button : buttons) {
        button->setMinimumWidth(maxRequiredWidth);
        button->updateGeometry();
    }

    // Reset group box size constraints
    groupBox->setMinimumWidth(0);
    groupBox->setMaximumWidth(QWIDGETSIZE_MAX);

    // Calculate if we need to expand the group box
    int rightmostButtonEdge = 0;
    for (QPushButton* button : buttons) {
        int buttonRight = button->x() + button->width();
        if (buttonRight > rightmostButtonEdge) {
            rightmostButtonEdge = buttonRight;
        }
    }

    // Add margin for group box
    int requiredGroupBoxWidth = rightmostButtonEdge + 20;

    // Set minimum width for group box
    if (requiredGroupBoxWidth > 0) {
        groupBox->setMinimumWidth(requiredGroupBoxWidth);
        groupBox->updateGeometry();
    }
}

// Recursively adjust all group boxes and their buttons
static void autoAdjustAllWidgetsForTranslation(QWidget* parent)
{
    if (!parent) return;

    // Find all QPushButton widgets and call adjustSize() on them
    // This allows buttons to resize based on their text content
    QList<QPushButton*> buttons = parent->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        // Only adjust buttons that don't have a fixed size policy
        if (button->sizePolicy().horizontalPolicy() != QSizePolicy::Fixed) {
            button->adjustSize();
        }
    }

    // Trigger a layout update to propagate changes
    if (parent->layout()) {
        parent->layout()->update();
        parent->layout()->activate();
    }
    parent->updateGeometry();
}

#endif // TEXT_FIT_HELPERS_H
