#ifndef MONOTONIC_HPP
#define MONOTONIC_HPP

namespace filter_node
{

  //=============================================================================
  // Gazebo problems:
  // -- msg.header.stamp might be 0 (bootstrap)
  // -- msg.header.stamp might repeat over consecutive messages (bug)
  // https://bitbucket.org/osrf/gazebo/issues/1966/camera-output-appears-to-depend-on-real
  //=============================================================================

  template<typename N, typename M>
  class Valid
  {
    N node_;
    std::function<void(N, M)> process_;         // Process good messages
    rclcpp::Time curr_;                         // Stamp of current message
    rclcpp::Time prev_;                         // Stamp of previous message

    static bool valid(const rclcpp::Time &t)
    {
      return t.nanoseconds() > 0;
    }

  public:

    Valid(N node, std::function<void(N, M)> callback)
    {
      node_ = node;
      process_ = callback;
    }

    void call(M msg)
    {
      curr_ = msg->header.stamp;

      if (valid(curr_)) {
        process_(node_, msg);
        prev_ = curr_;
      }
    }

    const rclcpp::Time &curr() const
    { return curr_; };

    const rclcpp::Time &prev() const
    { return prev_; };

    double dt() const
    { return (curr() - prev()).seconds(); }

    bool receiving() const
    { return valid(prev_); }
  };

  template<typename N, typename M>
  class Monotonic
  {
    N node_;
    std::function<void(N, M, bool)> process_;   // Process good messages
    rclcpp::Time curr_;                         // Stamp of current message
    rclcpp::Time prev_;                         // Stamp of previous message

    static bool valid(const rclcpp::Time &t)
    {
      return t.nanoseconds() > 0;
    }

  public:

    Monotonic(N node, std::function<void(N, M, bool)> callback)
    {
      node_ = node;
      process_ = callback;
    }

    void call(M msg)
    {
      curr_ = msg->header.stamp;

      if (valid(curr_)) {
        if (valid(prev_)) {
          // Must be monotonic
          if (curr_ > prev_) {
            process_(node_, msg, false);
            prev_ = curr_;
          }
        } else {
          process_(node_, msg, true);
          prev_ = curr_;
        }
      }
    }

    const rclcpp::Time &curr() const
    { return curr_; };

    const rclcpp::Time &prev() const
    { return prev_; };

    double dt() const
    { return (curr() - prev()).seconds(); }

    bool receiving() const
    { return valid(prev_); }
  };

} // namespace filter_node

#endif // MONOTONIC_HPP
