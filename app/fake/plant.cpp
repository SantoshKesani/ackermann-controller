/* @file plant.cpp
 * @copyright [2020]
 */

#include <fake/plant.h>
#include <iostream>

namespace fake {

Plant::Plant(const PlantOptions& opts,
             const std::shared_ptr<const ackermann::Params>& params)
  : opts_(opts),
    params_(params),
    dist_(opts.noise_mean, opts.noise_stddev),
    limits_(std::make_unique<ackermann::Limits>(params)) {
  this->reset();
}

void Plant::reset() {
  speed_ = 0.0;
  heading_ = 0.0;
}

void Plant::setState(const double speed, const double heading) {
  speed_ = speed;
  heading_ = limits_->boundHeading(heading);
}

void Plant::getState(double& speed, double& heading) const {
  speed = speed_;
  heading = heading_;
}

void Plant::command(const double throttle,
                    const double steering,
                    const double dt) {
  // sanity check inputs
  double steering_capped = steering;
  if (std::abs(steering) > opts_.max_steering_angle) {
    steering_capped = ((steering > 0) - (steering < 0)) *
                      opts_.max_steering_angle;
  }

  // throttle translates directly to speed, since we have no other system
  // knowledge
  speed_ = limits_->throttleToSpeed(throttle);

  // the global heading is affected by our speed, steering angle, and wheel base
  // heading_ += dt * (speed_ / opts_.wheel_base) * std::tan(steering_capped);
  this->heading_ = limits_->boundHeading(this->heading_
                   + ((this->speed_/params_->wheel_base)
                      * tan(steering_capped) * dt));

  // add in some noise for good measure
  speed_ += dist_(generator_);
  heading_ += dist_(generator_);
}

}  // namespace fake
