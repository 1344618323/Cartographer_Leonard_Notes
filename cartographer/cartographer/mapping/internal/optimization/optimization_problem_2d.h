/*
 * Copyright 2016 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CARTOGRAPHER_MAPPING_INTERNAL_OPTIMIZATION_OPTIMIZATION_PROBLEM_2D_H_
#define CARTOGRAPHER_MAPPING_INTERNAL_OPTIMIZATION_OPTIMIZATION_PROBLEM_2D_H_

#include <array>
#include <deque>
#include <map>
#include <set>
#include <vector>

#include "Eigen/Core"
#include "Eigen/Geometry"
#include "cartographer/common/port.h"
#include "cartographer/common/time.h"
#include "cartographer/mapping/id.h"
#include "cartographer/mapping/internal/optimization/optimization_problem_interface.h"
#include "cartographer/mapping/pose_graph_interface.h"
#include "cartographer/mapping/proto/pose_graph/optimization_problem_options.pb.h"
#include "cartographer/sensor/imu_data.h"
#include "cartographer/sensor/map_by_time.h"
#include "cartographer/sensor/odometry_data.h"
#include "cartographer/transform/timestamped_transform.h"

namespace cartographer {
namespace mapping {
namespace optimization {

struct NodeSpec2D {
  common::Time time;
  transform::Rigid2d local_pose_2d;
  transform::Rigid2d global_pose_2d;
  Eigen::Quaterniond gravity_alignment;
};

struct SubmapSpec2D {
  transform::Rigid2d global_pose;
};

class OptimizationProblem2D
    : public OptimizationProblemInterface<NodeSpec2D, SubmapSpec2D,
                                          transform::Rigid2d> {
 public:
  explicit OptimizationProblem2D(
      const optimization::proto::OptimizationProblemOptions& options);
  ~OptimizationProblem2D();

  OptimizationProblem2D(const OptimizationProblem2D&) = delete;
  OptimizationProblem2D& operator=(const OptimizationProblem2D&) = delete;

  void AddImuData(int trajectory_id, const sensor::ImuData& imu_data) override;
  void AddOdometryData(int trajectory_id,
                       const sensor::OdometryData& odometry_data) override;
  void AddTrajectoryNode(int trajectory_id,
                         const NodeSpec2D& node_data) override;
  void InsertTrajectoryNode(const NodeId& node_id,
                            const NodeSpec2D& node_data) override;
  void TrimTrajectoryNode(const NodeId& node_id) override;
  void AddSubmap(int trajectory_id,
                 const transform::Rigid2d& global_submap_pose) override;
  void InsertSubmap(const SubmapId& submap_id,
                    const transform::Rigid2d& global_submap_pose) override;
  void TrimSubmap(const SubmapId& submap_id) override;
  void SetMaxNumIterations(int32 max_num_iterations) override;

  void Solve(
      const std::vector<Constraint>& constraints,
      const std::set<int>& frozen_trajectories,
      const std::map<std::string, LandmarkNode>& landmark_nodes) override;

  const MapById<NodeId, NodeSpec2D>& node_data() const override {
    return node_data_;
  }
  const MapById<SubmapId, SubmapSpec2D>& submap_data() const override {
    return submap_data_;
  }
  const std::map<std::string, transform::Rigid3d>& landmark_data()
      const override {
    return landmark_data_;
  }
  const sensor::MapByTime<sensor::ImuData>& imu_data() const override {
    return imu_data_;
  }
  const sensor::MapByTime<sensor::OdometryData>& odometry_data()
      const override {
    return odometry_data_;
  }

 private:
  std::unique_ptr<transform::Rigid3d> InterpolateOdometry(
      int trajectory_id, common::Time time) const;
  // Computes the relative pose between two nodes based on odometry data.
  std::unique_ptr<transform::Rigid3d> CalculateOdometryBetweenNodes(
      int trajectory_id, const NodeSpec2D& first_node_data,
      const NodeSpec2D& second_node_data) const;

  optimization::proto::OptimizationProblemOptions options_;
  MapById<NodeId, NodeSpec2D> node_data_;
  MapById<SubmapId, SubmapSpec2D> submap_data_;
  std::map<std::string, transform::Rigid3d> landmark_data_;
  sensor::MapByTime<sensor::ImuData> imu_data_;
  sensor::MapByTime<sensor::OdometryData> odometry_data_;
  //(cxn)对某一类型的传感器，所有轨迹的数据都会存到 MapByTime，这个类里有个map<int,map<Time,Data>>
  // int是轨迹编号，Time是数据时间，Data是数据
};

}  // namespace optimization
}  // namespace mapping
}  // namespace cartographer

#endif  // CARTOGRAPHER_MAPPING_INTERNAL_OPTIMIZATION_OPTIMIZATION_PROBLEM_2D_H_
