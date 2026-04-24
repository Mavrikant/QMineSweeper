#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <QDialog>
#include <QVector>

class QLabel;
class QPushButton;

// First-run walkthrough for new users — a small modal card with Back /
// Next / Skip that shows a short sequence of steps covering the core
// mechanics (reveal, numbers, flag/?, chord, where to find the menus).
// Manually re-openable via Help → Tutorial at any time.
namespace Tutorial
{
// Raw string literals marked with QT_TR_NOOP so lupdate extracts them
// even though we hand them to tr() at the use site.
struct Step
{
    const char *title;
    const char *body;
};

[[nodiscard]] const QVector<Step> &steps();

[[nodiscard]] bool isCompleted();
void markCompleted();
void clearCompleted(); // test + Help-menu reset-on-reopen paths
} // namespace Tutorial

class TutorialDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit TutorialDialog(QWidget *parent = nullptr);

    [[nodiscard]] int currentStep() const noexcept;

  signals:
    // Emitted when the user reaches the final step and clicks Finish.
    void completed();
    // Emitted when the user dismisses the dialog via Skip (or the close box).
    void skipped();

  protected:
    void closeEvent(QCloseEvent *e) override;

  private:
    void render();
    void goNext();
    void goBack();

    int m_index{0};
    QLabel *m_stepLabel{nullptr};
    QLabel *m_titleLabel{nullptr};
    QLabel *m_bodyLabel{nullptr};
    QPushButton *m_backBtn{nullptr};
    QPushButton *m_nextBtn{nullptr};
    QPushButton *m_skipBtn{nullptr};
    bool m_finished{false};
};

#endif // TUTORIAL_H
